/*
 * @Brief
 * This example shows how to connect an ESP device to the Bytebeam cloud.
 * This is good starting point if you are looking to remotely update your ESP device
 * or push a command to it or stream data from it and visualise it on the Bytebeam Cloud.
 */

#include <math.h>
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
#include "driver/ledc.h"
#include "bytebeam_sdk.h"

// this macro is used to specify the delay for 1 sec.
#define APP_DELAY_ONE_SEC 1000u

// this macro is used to specify the maximum length of led status string
#define LED_STATUS_STR_LEN 200

// this macro is used to specify the gpio led for update config action
#define UPDATE_GPIO 2

#define LEDC_TIMER              LEDC_TIMER_0            // ledc timer 0 peripheral  
#define LEDC_MODE               LEDC_LOW_SPEED_MODE     // ledc low speed mode 
#define LEDC_OUTPUT_IO          UPDATE_GPIO             // define the output gpio
#define LEDC_CHANNEL            LEDC_CHANNEL_0          // ledc channel0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT       // set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000)                  // frequency in hz Set frequency at 5 kHz
#define LEDC_STEP_SIZE          255                     // set the step size with 8 bits resolution

static int config_update_period = APP_DELAY_ONE_SEC;

static uint32_t led_duty_cycle = 0;
static int update_config_cmd = 0;

static char led_status[LED_STATUS_STR_LEN] = "";
static char device_shadow_stream[] = "device_shadow";

static bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_UPDATE_CONFIG_EXAMPLE";

static void set_led_status(void)
{
    int max_len = LED_STATUS_STR_LEN;
    uint32_t ledc_res = (uint32_t)round(pow(2,LEDC_DUTY_RES)) - 1;
    float brightness = (led_duty_cycle/(float)ledc_res) * 100;

    int temp_var = snprintf(led_status, max_len, "LED Brighntness is %.2f%% !", brightness);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "led status string size exceeded max length of buffer");
    }
}

static void update_led(void) 
{
    // set the new duty cycle for update gpio led
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, led_duty_cycle);
    ESP_LOGI(TAG, " LED Duty Cycle Value is %d!", (int)led_duty_cycle);

    // update the new duty cycle for update gpio led
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void ledc_init(void)
{
    // prepare and then apply the ledc pwm timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // prepare and then apply the ledc pwm channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
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

    device_status_json = cJSON_CreateString(led_status);

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
        if (update_config_cmd == 1)
        {
            // update the led
            update_led();
            
            // set led status
            set_led_status();

            // pulish led status to device shadow
            ret_val = publish_device_shadow(bytebeam_client);

            if (ret_val != 0)
            {
                ESP_LOGE(TAG, "Publish to device shadow failed");
            }

            // reset the update config command flag
            update_config_cmd = 0;
        }

        vTaskDelay(config_update_period / portTICK_PERIOD_MS);
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

// handler for update config action
int handle_update_config(bytebeam_client_t *bytebeam_client, char *args, char *action_id) 
{
    int ret_val = 0;
    cJSON *root = NULL;
    cJSON *name = NULL;
    cJSON *version = NULL;
    cJSON *step_value = NULL;

    // parse the received json
    root = cJSON_Parse(args);

    if (root == NULL) 
    {
        ESP_LOGE(TAG, "ERROR in parsing the JSON\n");

        // publish action failed response
        ret_val = bytebeam_publish_action_failed(bytebeam_client, action_id);

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Failed to Publish action failed response for Update Config action");
        }

        return -1;
    }

    name = cJSON_GetObjectItem(root, "name");

    if (!(cJSON_IsString(name) && (name->valuestring != NULL))) 
    {
        ESP_LOGE(TAG, "Error parsing update config name\n");

        // publish action failed response
        ret_val = bytebeam_publish_action_failed(bytebeam_client, action_id);

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Failed to Publish action failed response for Update Config action");
        }

        cJSON_Delete(root);
        return -1;
    }

    ESP_LOGI(TAG, "Checking update config name \"%s\"\n", name->valuestring);

    version = cJSON_GetObjectItem(root, "version");

    if (!(cJSON_IsString(version) && (version->valuestring != NULL)))
    {
        ESP_LOGE(TAG, "Error parsing update config version\n");

        // publish action failed response
        ret_val = bytebeam_publish_action_failed(bytebeam_client, action_id);

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Failed to Publish action failed response for Update Config action");
        }

        cJSON_Delete(root);
        return -1;
    }

    ESP_LOGI(TAG, "Checking update config version \"%s\"\n", version->valuestring);

    step_value = cJSON_GetObjectItem(root, "step_value");

    if (!(cJSON_IsNumber(step_value)))
    {
        ESP_LOGE(TAG, "Error parsing update config step value\n");

        // publish action failed response
        ret_val = bytebeam_publish_action_failed(bytebeam_client, action_id);

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Failed to Publish action failed response for Update Config action");
        }

        cJSON_Delete(root);
        return -1;
    }

    ESP_LOGI(TAG, "Checking update config step value %d\n", (int)step_value->valuedouble);

    // Generate the duty cycle for the led
    uint32_t ledc_res = (uint32_t)round(pow(2,LEDC_DUTY_RES)) - 1;
    led_duty_cycle = ((ledc_res) * (step_value->valuedouble/LEDC_STEP_SIZE));

    // set the update config command flag
    update_config_cmd = 1;

    // publish action completed response
    ret_val = bytebeam_publish_action_completed(bytebeam_client, action_id);

    if (ret_val != 0)
    {
        ESP_LOGE(TAG, "Failed to Publish action completed response for Update Config action");
		return -1;
    }

    // release the memory occupied by the json object
    cJSON_Delete(root);

    return 0;
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

    // configure ledc peripheral
    ledc_init();

    // initialize the bytebeam client
    bytebeam_init(&bytebeam_client);

    // add the handler for update config action
    bytebeam_add_action_handler(&bytebeam_client, handle_update_config, "update_config");

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}
