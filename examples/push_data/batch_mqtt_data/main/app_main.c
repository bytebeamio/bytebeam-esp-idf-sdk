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

// this macro is used to specify the delay for 200 ms
#define APP_DELAY_TWO_HUNDRED_MS 200u

static int config_delay_period = APP_DELAY_TWO_HUNDRED_MS;

static bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_BATCH_MQTT_DATA_EXAMPLE";

float accel_x = -1.1;
float accel_y = 0.7;
float accel_z = -1.7;
float gyro_x  = 1.1;
float gyro_y  = 0.33;
float gyro_z  = 0.78;

static int publish_acc_gyro_values()
{
    struct timeval te;
    long long milliseconds = 0;
    static uint64_t sequence = 0;

    cJSON *acc_gyro_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *accel_x_json = NULL;
    cJSON *accel_y_json = NULL;
    cJSON *accel_z_json = NULL;
    cJSON *gyro_x_json = NULL;
    cJSON *gyro_y_json = NULL;
    cJSON *gyro_z_json = NULL;

    char *string_json = NULL;

    acc_gyro_json = cJSON_CreateObject();

    if (acc_gyro_json == NULL)
    {
        ESP_LOGE(TAG, "Json add failed.");
        return -1;
    }

    // get current time
    gettimeofday(&te, NULL);
    milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    // make sure you got the epoch millis
    if(milliseconds == 0)
    {
        ESP_LOGE(TAG, "failed to get epoch millis.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if (timestamp_json == NULL)
    {
        ESP_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if (sequence_json == NULL)
    {
        ESP_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "sequence", sequence_json);

    accel_x_json = cJSON_CreateNumber(accel_x);

    if (accel_x_json == NULL)
    {
        ESP_LOGE(TAG, "Json add accel x value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "accel_x", accel_x_json);

    accel_y_json = cJSON_CreateNumber(accel_y);

    if (accel_y_json == NULL)
    {
        ESP_LOGE(TAG, "Json add accel y value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "accel_y", accel_y_json);

    accel_z_json = cJSON_CreateNumber(accel_z);

    if (accel_z_json == NULL)
    {
        ESP_LOGE(TAG, "Json add accel z value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "accel_z", accel_z_json);

    gyro_x_json = cJSON_CreateNumber(gyro_x);

    if (gyro_x_json == NULL)
    {
        ESP_LOGE(TAG, "Json add gyro x value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "gyro_x", gyro_x_json);

    gyro_y_json = cJSON_CreateNumber(gyro_y);

    if (gyro_y_json == NULL)
    {
        ESP_LOGE(TAG, "Json add gyro y value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "gyro_y", gyro_y_json);

    gyro_z_json = cJSON_CreateNumber(gyro_z);

    if (gyro_z_json == NULL)
    {
        ESP_LOGE(TAG, "Json add gyro z value failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    cJSON_AddItemToObject(acc_gyro_json, "gyro_z", gyro_z_json);

    string_json = cJSON_Print(acc_gyro_json);

    if(string_json == NULL)
    {
        ESP_LOGE(TAG, "Json string print failed.");
        cJSON_Delete(acc_gyro_json);
        return -1;
    }

    // publish the accel gyro json to acc_gyro stream
    int ret_val = bytebeam_batch_publish_to_stream(string_json);

    cJSON_Delete(acc_gyro_json);
    cJSON_free(string_json);

    return ret_val;
}

static void app_start(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0; 

    while (1) 
    {
        // publish esp touch values
        ret_val = publish_acc_gyro_values();

        if (ret_val != 0)
        {
            ESP_LOGE(TAG, "Failed to publish acc gyro values.");
        }

        vTaskDelay(config_delay_period / portTICK_PERIOD_MS);
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

    // initialze the bytebeam batch handle
    bytebeam_batch_init(&bytebeam_client, "acc_gyro");

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}