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

#include "bytebeam_sdk.h"

// this macro is used to specify the delay for 10 sec.
#define APP_DELAY_TEN_SEC 10000u

static int config_delay_period = APP_DELAY_TEN_SEC;

static bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_CLOUD_LOGGING_EXAMPLE";

static void bytebeam_cloud_logging_test(void) 
{
    /* default bytebeam log level is BYTEBEAM_LOG_LEVEL_INFO, so logs beyond info level will not work :) 
     * You can always change the log setting at the compile time or run time
     */
    BYTEBEAM_LOGE(TAG, "I am %s Log", "Error");
    BYTEBEAM_LOGW(TAG, "I am %s Log", "Warn");
    BYTEBEAM_LOGI(TAG, "I am %s Log", "Info");
    BYTEBEAM_LOGD(TAG, "I am %s Log", "Debug");                   // debug Log will not appear to cloud
    BYTEBEAM_LOGV(TAG, "I am %s Log", "Verbose");                 // verbose Log will not appear to cloud

    // changing log level to Verbose for showing use case   
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL_VERBOSE); 

    /* now bytebeam log level is BYTEBEAM_LOG_LEVEL_VERBOSE, so every logs should work now :)
     * make sure your esp log level supports Verbose to see the log in the terminal
     */
    BYTEBEAM_LOGE(TAG, "This is %s Log", "Error");
    BYTEBEAM_LOGW(TAG, "This is %s Log", "Warn");
    BYTEBEAM_LOGI(TAG, "This is %s Log", "Info");
    BYTEBEAM_LOGD(TAG, "This is %s Log", "Debug");                // debug Log should appear to cloud
    BYTEBEAM_LOGV(TAG, "This is %s Log", "Verbose");              // verbose Log should appear to cloud

    // changing log level back to Info for meeting initial conditions
    esp_log_level_set(TAG, ESP_LOG_INFO);
    bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL_INFO);

    ESP_LOGI(TAG, "Bytebeam Log Test Executed Successfully !\n");
}

static void app_start(bytebeam_client_t *bytebeam_client)
{
    while (1) 
    {
        //
        // nothing much to do here at the moment
        //

        if(bytebeam_client->connection_status == 1)
        {
            // this is how you can do cloud logging, try other log levels for better understanding
            BYTEBEAM_LOGI(TAG, "Status : Connected");
            BYTEBEAM_LOGI(TAG, "Project Id : %s, Device Id : %s", bytebeam_client->device_cfg.project_id, bytebeam_client->device_cfg.device_id);
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

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    // check if cloud logging is enabled or disabled for your device
    bool cloud_logging_status = bytebeam_is_cloud_logging_enabled();

    if(cloud_logging_status) {
        ESP_LOGI(TAG, "Cloud Logging is Enabled.");
    } else {
        ESP_LOGI(TAG, "Cloud Logging is Disabled.");
    }

    // enable cloud logging for your device (default)
    // bytebeam_enable_cloud_logging();

    // disable cloud logging for your device
    // bytebeam_disable_cloud_logging();

    // get the log stream name
    // char* log_stream_name = bytebeam_log_stream_get();

    // configure the log stream if needed, defaults to "logs"
    // bytebeam_log_stream_set("device_logs");

    // get the bytebeam log level
    // int current_log_level = bytebeam_log_level_get();

    // set the bytebeam log level
    // bytebeam_log_level_set(log_level_to_set);

    /* bytebeam logs can be tested once bytebeam client is started successfully, Use bytebeam_cloud_logging_test to test
     * the bytebeam log feature i.e this functions is not included in the sdk
     */
    // bytebeam_cloud_logging_test();

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}