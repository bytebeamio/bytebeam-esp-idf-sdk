/**
 * Brief
 * This example shows how to connect an ESP device to the Bytebeam cloud.
 * This is good starting point if you are looking to remotely update your ESP device
   or push a command to it or stream data from it and visualise it on the Bytebeam Cloud.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_tls.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "cJSON.h"

#include "driver/gpio.h"

#include "bytebeam_sdk.h"
// #include "bytebeam_esp_hal.h"
// #include "bytebeam_actions.h"

#define BLINK_GPIO 2

static uint8_t led_state = 0;
static int config_blink_period = 1000;
static int toggle_led_cmd = 0;

bytebeam_client bb_obj;

static const char *TAG = "BYTEBEAM_DEMO_EXAMPLE";

// bytebeam_client bb_obj;

static void blink_led(void)
{
	/* Set the GPIO level according to the state (LOW or HIGH)*/
	gpio_set_level(BLINK_GPIO, led_state);
}

static void configure_led(void)
{
	gpio_reset_pin(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

static int publish_device_shadow(bytebeam_client *bb_obj)
{
	static uint64_t sq_num = 0;
	cJSON *device_shadow_json_list = NULL;
	cJSON *device_shadow_json = NULL;
	cJSON *seq_id_json = NULL;
	cJSON *timestamp_json = NULL;
	cJSON *device_status_json = NULL;
	char *string_json = NULL;
	char status_update[800] = {
		0,
	};

	device_shadow_json_list = cJSON_CreateArray();

	if (device_shadow_json_list == NULL)
	{
		ESP_LOGE(TAG, "Json Init failed.");
		return -1;
	}

	device_shadow_json = cJSON_CreateObject();

	if (device_shadow_json == NULL)
	{
		ESP_LOGE(TAG, "Json add failed.");
		cJSON_Delete(device_shadow_json_list);
		return -1;
	}

	struct timeval te;
	sq_num++;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
	timestamp_json = cJSON_CreateNumber(milliseconds);

	if (timestamp_json == NULL)
	{
		ESP_LOGE(TAG, "Json add time stamp failed.");
		cJSON_Delete(device_shadow_json_list);
		return -1;
	}

	cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

	seq_id_json = cJSON_CreateNumber(sq_num);

	if (seq_id_json == NULL)
	{
		ESP_LOGE(TAG, "Json add sequence id failed.");
		cJSON_Delete(device_shadow_json_list);
		return -1;
	}

	cJSON_AddItemToObject(device_shadow_json, "sequence", seq_id_json);

	char temp_buff[200];
	sprintf(temp_buff, "LED is %s!", led_state == true ? "ON" : "OFF");
	device_status_json = cJSON_CreateString(temp_buff);

	if (device_status_json == NULL)
	{
		ESP_LOGE(TAG, "Json add device status failed.");
		cJSON_Delete(device_shadow_json_list);
		return -1;
	}

	cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

	cJSON_AddItemToArray(device_shadow_json_list, device_shadow_json);

	string_json = cJSON_Print(device_shadow_json_list);
	sprintf(status_update, "%s", string_json);
	ESP_LOGI(TAG, "\nStatus to send:\n%s\n", status_update);

	bytebeam_publish_to_stream(bb_obj, "device_shadow", string_json);

	cJSON_Delete(device_shadow_json_list);
	free(string_json);
	return 0;
}

static void app_start(bytebeam_client *bb_obj)
{
	int ret_val = 0;

	while (1)
	{
		ret_val = publish_device_shadow(bb_obj);

		if (ret_val != 0)
		{
			ESP_LOGE(TAG, "TERMINATING PUBLISH DUE TO FAILURE IN JSON CREATION");
		}

		if (toggle_led_cmd == 1)
		{
			/* Toggle the LED state */
			led_state = !led_state;
			ESP_LOGI(TAG, " LED_%s!", led_state == true ? "ON" : "OFF");
			blink_led();
			toggle_led_cmd = 0;
		}

		vTaskDelay(config_blink_period / portTICK_PERIOD_MS);
	}
}

static void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
	ESP_LOGI(TAG, "Initializing SNTP");

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
	sntp_init();
}

static void obtain_time(void)
{
	initialize_sntp();
	// wait for time to be set
	time_t now = 0;
	struct tm timeinfo = {0};
	int retry = 0;
	const int retry_count = 10;

	while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
	{
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

	time(&now);
	localtime_r(&now, &timeinfo);
}

int toggle_led(bytebeam_client *bb_obj, char *args, char *action_id)
{
	toggle_led_cmd = 1;
	if ((bytebeam_publish_action_completed(bb_obj, action_id)) != 0)
	{
		ESP_LOGE(TAG, "Failed to Publish action response for Toggle LED action");
	}
	return 0;
}

void app_main(void)
{
	ESP_LOGI(TAG, "[APP] Startup..");
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
	esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	/* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
	 * Read "Establishing Wi-Fi or Ethernet Connection" section in
	 * examples/protocols/README.md for more information about this function.
	 */
	ESP_ERROR_CHECK(example_connect());

	obtain_time();
	configure_led();
	bytebeam_init(&bb_obj);
	bytebeam_create_new_action_handler(&bb_obj, handle_ota, "update_firmware");
	bytebeam_create_new_action_handler(&bb_obj, toggle_led, "toggle_board_led");
	bytebeam_start(&bb_obj);
	app_start(&bb_obj);
}
