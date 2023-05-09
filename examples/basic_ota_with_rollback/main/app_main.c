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

#include "esp_ota_ops.h"

#include "bytebeam_sdk.h"

// this macro is used to specify the delay for 10 sec.
#define APP_DELAY_TEN_SEC 10000u

// this macro is used to specify the firmware version
#define APP_FIRMWARE_VERSION "1.0.0"

static int config_delay_period = APP_DELAY_TEN_SEC;

static const char *fw_version = APP_FIRMWARE_VERSION;

static bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE";

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

static bool diagnostic(void)
{
    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    // your checkpoint will go here i.e (cloud connection status)
    bool diagnostic_is_ok = bytebeam_client.connection_status;

    return diagnostic_is_ok;
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

    // setting up the device info i.e to be seen in the device shadow
    bytebeam_client.device_info.status           = "Device is Up!";
    bytebeam_client.device_info.software_type    = "basic-ota-app";
    bytebeam_client.device_info.software_version = fw_version;
    bytebeam_client.device_info.hardware_type    = "ESP32 DevKit V1";
    bytebeam_client.device_info.hardware_version = "rev1";

    // initialize the bytebeam client
    bytebeam_init(&bytebeam_client);

    // add the handler for update firmware action i.e handling ota is internal to the sdk
    bytebeam_add_action_handler(&bytebeam_client, handle_ota, "update_firmware");

    /* Use the bytebeam_remove_action_handler api to remove the handler for the update firmware action at any point of time
     * in the code , Also you can  mantain the add update firmware action handler and remove update firmware action handler
     * flow as per you application needs.
     */
    // bytebeam_remove_action_handler(&bytebeam_client, "update_firmware");

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);
    
    ESP_LOGI(TAG, "Application Firmware Version : %s", fw_version);

#if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE)
    /**
     * We are treating successful cloud connection as a checkpoint to cancel rollback
     * process and mark newly updated firmware image as active. For production cases,
     * please tune the checkpoint behavior per end application requirement.
     */

    esp_err_t ret_code;
    esp_ota_img_states_t ota_state;
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
            bool diagnostic_is_ok = diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");

                ret_code = esp_ota_mark_app_valid_cancel_rollback();

                if (ret_code == ESP_OK) {
                    ESP_LOGI(TAG, "Application marked as valid and rollback cancelled successfully");
                } else {
                    ESP_LOGE(TAG, "Failed to cancel rollback (%s)", esp_err_to_name(ret_code));
                }
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                
                // on success it will not not retrun instead it boot the previous app
                // so just throw the error log in case of failure
                ret_code = esp_ota_mark_app_invalid_rollback_and_reboot();
                ESP_LOGE(TAG, "Failed to cancel rollback (%s)", esp_err_to_name(ret_code));
            }
        }
    }
#endif

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}