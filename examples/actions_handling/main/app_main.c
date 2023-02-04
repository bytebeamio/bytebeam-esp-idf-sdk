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

static const char *TAG = "BYTEBEAM_SETUP_CLIENT_EXAMPLE";

static void app_start(bytebeam_client_t *bytebeam_client)
{
    while (1) 
    {
        //
        // nothing much to do here at the moment
        //

        if(bytebeam_client->connection_status == 1)
        {
            ESP_LOGI(TAG, "Status : Connected");
            ESP_LOGI(TAG, "Project Id : %s, Device Id : %s", bytebeam_client->device_cfg.project_id, bytebeam_client->device_cfg.device_id);
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

// handler for hello world action
int handle_hello_world(bytebeam_client_t *bytebeam_client, char *args, char *action_id)
{
    //
    // nothing much to do here at the moment
    //

    ESP_LOGI(TAG, "Hello World !");

    return 0;
}

// yet another handler for hello world action
int handle_yet_another_hello_world(bytebeam_client_t *bytebeam_client, char *args, char *action_id)
{
    //
    // nothing much to do here at the moment
    //

    ESP_LOGI(TAG, "Yet Another Hello World !");

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

    // initialize the bytebeam client
    bytebeam_init(&bytebeam_client);

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    // add the handlerfor hello world action
    bytebeam_add_action_handler(&bytebeam_client, handle_hello_world, "hello_world");

    // Use the bytebeam_is_action_handler_there api to check if particular action exists or not at any point of time in the code
    // bytebeam_is_action_handler_there(&bytebeam_client, "hello_world");

    // Use the bytebeam_update_action_handler api to update the particular action at any point of time in the code
    // bytebeam_update_action_handler(&bytebeam_client, handle_yet_another_hello_world, "hello_world");

    // Use the bytebeam_print_action_handler_array api to print the action handler array at any point of time in the code
    // bytebeam_print_action_handler_array(&bytebeam_client);

    // Use the bytebeam_remove_action_handler api to remove the particular action at any point of time in the code
    // bytebeam_remove_action_handler(&bytebeam_client, "hello_world");

    // Use the bytebeam_reset_action_handler_array api to reset the action handler array at any point of time in the code
    // bytebeam_reset_action_handler_array(&bytebeam_client);

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}