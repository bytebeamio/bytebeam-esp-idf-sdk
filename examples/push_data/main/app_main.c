/*
 * @Brief
 * This example shows how to connect an ESP device to the Bytebeam cloud.
 * This is good starting point if you are looking to remotely update your ESP device
 * or push a command to it or stream data from it and visualise it on the Bytebeam Cloud.
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

#include "esp_sntp.h"
#include "esp_log.h"

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"
#include "bytebeam_sdk.h"

// this macro is used to specify the delay for 10 sec.
#define APP_DELAY_TEN_SEC 10000u

// this macro is used to specify the maximum length of device status string
#define DEVICE_STATUS_STR_LEN 200

static int config_status_period = APP_DELAY_TEN_SEC;

static char device_status[DEVICE_STATUS_STR_LEN] = "";
static char device_shadow_stream[] = "device_shadow";

static bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_TOGGLE_LED_EXAMPLE";

static void set_device_status(void)
{
    int max_len = DEVICE_STATUS_STR_LEN;
    int temp_var = snprintf(device_status, max_len, "Device Status %s !", "Working");

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "device status string size exceeded max length of buffer");
    }
}

static int publish_device_shadow(bytebeam_client_t *bytebeam_client)
{   
    struct timeval te;
    long long milliseconds = 0;
    static uint64_t sequence = 0;

    cJSON *device_shadow_json_list = NULL;
    cJSON *device_shadow_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *device_status_json = NULL;

    char *string_json = NULL;

    device_shadow_json_list = cJSON_CreateArray();

    if(device_shadow_json_list == NULL)
    {
        ESP_LOGE(TAG, "Json Init failed.");
        return -1;
    }

    device_shadow_json = cJSON_CreateObject();

    if(device_shadow_json == NULL)
    {
        ESP_LOGE(TAG, "Json add failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    // get the current time
    gettimeofday(&te, NULL); 
    milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    // make sure you got the epoch millis
    if(milliseconds == 0)
    {
        ESP_LOGE(TAG, "failed to get epoch millis.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if(timestamp_json == NULL)
    {
        ESP_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if(sequence_json == NULL)
    {
        ESP_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "sequence", sequence_json);

    device_status_json = cJSON_CreateString(device_status);

    if(device_status_json == NULL)
    {
        ESP_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    cJSON_AddItemToArray(device_shadow_json_list, device_shadow_json);

    string_json = cJSON_Print(device_shadow_json_list);

    if(string_json == NULL)
    {
        ESP_LOGE(TAG, "Json string print failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    ESP_LOGI(TAG, "\nStatus to send:\n%s\n", string_json);

    // publish the json to device shadow stream
    int ret_val = bytebeam_publish_to_stream(bytebeam_client, device_shadow_stream, string_json);

    cJSON_Delete(device_shadow_json_list);
    cJSON_free(string_json);

    return ret_val;
}

static void app_start(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;

    while (1) 
    {
        // set device status
        set_device_status();

        // pulish device status to device shadow
        ret_val = publish_device_shadow(bytebeam_client);

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Publish to device shadow failed");
        }

        vTaskDelay(config_status_period / portTICK_PERIOD_MS);
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

static void sync_time_from_ntp(void)
{
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    initialize_sntp();

    // wait for time to be set
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) 
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    time(&now);
    localtime_r(&now, &timeinfo);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", (int)esp_get_free_heap_size());
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

    // sync time from the ntp
    sync_time_from_ntp();

    // initialize the bytebeam client
    bytebeam_init(&bytebeam_client);

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}