#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_https_ota.h"
#include "esp_idf_version.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "bytebeam_esp_hal.h"
#include "bytebeam_ota.h"
#include "bytebeam_action.h"
#include "bytebeam_stream.h"
#include "bytebeam_client.h"

static int ota_img_data_len = 0;
static int ota_update_completed = 0;
static char ota_action_id_str[BYTEBEAM_ACTION_ID_STR_LEN] = "";
static bytebeam_client_t *ota_client = NULL;

static const char *TAG = "BYTEBEAM_HAL";

int bytebeam_hal_mqtt_subscribe(bytebeam_client_handle_t client, char *topic, int qos)
{
    return esp_mqtt_client_subscribe(client, (const char *)topic, qos);
}

int bytebeam_hal_mqtt_unsubscribe(bytebeam_client_handle_t client, char *topic)
{
    return esp_mqtt_client_unsubscribe(client, (const char *)topic);
}

int bytebeam_hal_mqtt_publish(bytebeam_client_handle_t client, char *topic, char *message, int length, int qos)
{
    return esp_mqtt_client_publish(client, (const char *)topic, (const char *)message, length, qos, 1);
}

int bytebeam_hal_restart(void)
{
    esp_restart();
    return 0;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int loop_var = 0;
    static int update_progress_offset = 10;
    static int update_progress_percent = 0;
    static int downloaded_data_len = 0;
    static const char* update_progress_status = "Downloading";

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        BB_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        BB_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;

    case HTTP_EVENT_HEADER_SENT:
        BB_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;

    case HTTP_EVENT_ON_HEADER:
        BB_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_DATA:
        BB_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        downloaded_data_len = downloaded_data_len + evt->data_len;
        update_progress_percent = (((float)downloaded_data_len / (float)ota_img_data_len) * 100.00);

        if (update_progress_percent == loop_var) {
            BB_LOGD(TAG, "update_progress_percent : %d", update_progress_percent);

            // If we are done, change the status to downloaded
            if(update_progress_percent == 100) {
                update_progress_status = "Downloaded";
            }

            // publish the OTA progress status
            if(bytebeam_publish_action_status(ota_client, ota_action_id, update_progress_percent, update_progress_status, "") != 0) {
                BB_LOGE(TAG, "Failed to publish OTA progress status");
            }

            if (loop_var == 100) {
                // reset the varibales
                loop_var = 0;
                update_progress_percent = 0;
                downloaded_data_len = 0;
            } else {
                // mark the next progress stamp
                loop_var = loop_var + update_progress_offset;
            }
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        BB_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;

    case HTTP_EVENT_DISCONNECTED:
        BB_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
	
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    case HTTP_EVENT_REDIRECT:
        BB_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
#endif
    }

    return ESP_OK;
}

esp_err_t _test_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        BB_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_DATA:
        ota_img_data_len = ota_img_data_len + evt->data_len;
        break;
    default:
        break;
    }

    return ESP_OK;
}

int bytebeam_hal_ota(bytebeam_client_t *bytebeam_client, char *ota_url)
{
    // set the ota client reference to the incoming bytebeam client
    ota_client = bytebeam_client;

    esp_http_client_config_t config = {
        .url = ota_url,
        .cert_pem = (char *)ota_client->device_cfg.ca_cert_pem,
        .client_cert_pem = (char *)ota_client->device_cfg.client_cert_pem,
        .client_key_pem = (char *)ota_client->device_cfg.client_key_pem,
        .event_handler = _http_event_handler,
    };

    esp_http_client_config_t test_config = {
        .url = ota_url,
        .cert_pem = (char *)ota_client->device_cfg.ca_cert_pem,
        .client_cert_pem = (char *)ota_client->device_cfg.client_cert_pem,
        .client_key_pem = (char *)ota_client->device_cfg.client_key_pem,
        .event_handler = _test_event_handler,
    };

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
#endif

    ota_img_data_len = 0;

    esp_http_client_handle_t client = esp_http_client_init(&test_config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        BB_LOGI(TAG, "content_length = %d", ota_img_data_len);
    } else {
        return -1;
    }

    esp_http_client_cleanup(client);
    BB_LOGI(TAG, "The URL is:%s", config.url);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    err = esp_https_ota(&ota_config);

    if (err != ESP_OK) {
        int max_len = BYTEBEAM_OTA_ERROR_STR_LEN;
        int temp_var = snprintf(ota_error_str, max_len, "Error (%d): %s", err, esp_err_to_name(err));

        if (temp_var >= max_len) {
            BB_LOGE(TAG, "OTA error size exceeded buffer size");
        }

        BB_LOGI(TAG, "HTTP_UPDATE_FAILED %s", ota_error_str);
        return -1;
    }
#else
    err = esp_https_ota(&config);

    if (err != ESP_OK) {
        int max_len = BYTEBEAM_OTA_ERROR_STR_LEN;
        int temp_var = snprintf(ota_error_str, max_len, "Error (%d): %s", err, esp_err_to_name(err));

        if (temp_var >= max_len) {
            BB_LOGE(TAG, "OTA error size exceeded buffer size");
        }

        BB_LOGI(TAG, "HTTP_UPDATE_FAILED %s", ota_error_str);
        return -1;
    }
#endif

    return 0;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        BB_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    BB_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);

    esp_mqtt_event_handle_t event = event_data;
    bytebeam_client_handle_t client = event->client;
    bytebeam_client_t *bytebeam_client = handler_args;
    int msg_id, ret_val;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        BB_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = bytebeam_subscribe_to_actions(bytebeam_client->device_cfg, client);

        if (msg_id != -1) {
            BB_LOGI(TAG, "MQTT SUBSCRIBED!! Msg ID:%d", msg_id);
        } else {
            BB_LOGE(TAG, "MQTT SUBSCRIBE FAILED");
        }

        bytebeam_client->connection_status = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        BB_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        bytebeam_client->connection_status = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        BB_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        BB_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        BB_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        BB_LOGI(TAG, "MQTT_EVENT_DATA");
        BB_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        BB_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);

        ret_val = bytebeam_handle_actions(event->data, event->client, bytebeam_client);

        if (ret_val != 0) {
            BB_LOGE(TAG, "BYTEBEAM HANDLE ACTIONS FAILED");
        } else {
            BB_LOGI(TAG, "BYTEBEAM HANDLE ACTIONS SUCCESS!!");
        }

        break;

    case MQTT_EVENT_ERROR:
        BB_LOGI(TAG, "MQTT_EVENT_ERROR");

        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            BB_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }

        break;

    default:
        BB_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

int bytebeam_hal_init(bytebeam_client_t *bytebeam_client)
{
    int32_t update_flag;
    nvs_handle_t temp_nv_handle;
    esp_err_t err;

    BB_LOGI(TAG, "[APP] Free memory: %d bytes", (int)esp_get_free_heap_size());

    bytebeam_client->client = esp_mqtt_client_init(&bytebeam_client->mqtt_cfg);

    if (bytebeam_client->client == NULL) {
        BB_LOGE(TAG, "MQTT Client initialization failed");
        return -1;
    }

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    err = esp_mqtt_client_register_event(bytebeam_client->client, ESP_EVENT_ANY_ID, mqtt_event_handler, bytebeam_client);

    if (err != ESP_OK) {
        BB_LOGE(TAG, "Failed to register MQTT event callback");
        return -1;
    }

    err = nvs_flash_init();

    if(err != ESP_OK)
    {
        BB_LOGE(TAG, "NVS flash init failed.");
        return -1;
    }

    err = nvs_open("test_storage", NVS_READWRITE, &temp_nv_handle);

    if (err != ESP_OK) 
    {
        BB_LOGE(TAG, "Failed to open NVS Storage");
        return -1;
    }

    err = nvs_get_i32(temp_nv_handle, "update_flag", &update_flag);

    if (err == ESP_OK) {
        if (update_flag == 1) {
            esp_err_t temp_err;
            int32_t ota_action_id_val;

            /* Reset the OTA update flag in NVS */
            update_flag = 0;
            temp_err = nvs_set_i32(temp_nv_handle, "update_flag", update_flag);

            if (temp_err != ESP_OK) 
            {
                BB_LOGE(TAG, "Failed to reset the OTA update flag in NVS");
                return -1;
            }

            temp_err = nvs_commit(temp_nv_handle);

            if (temp_err != ESP_OK) 
            {
                BB_LOGE(TAG, "Failed to commit the OTA update flag in NVS");
                return -1;
            }

            /* Get the OTA action id from NVS */
            temp_err = nvs_get_i32(temp_nv_handle, "action_id_val", &ota_action_id_val);

            if (temp_err != ESP_OK) 
            {
                BB_LOGE(TAG, "Failed to retreieve the OTA action id from NVS");
                return -1;
            }

            int max_len = BYTEBEAM_ACTION_ID_STR_LEN;
            int temp_var = snprintf(ota_action_id_str, max_len, "%d", (int)ota_action_id_val);

            if(temp_var >= max_len)
            {
                BB_LOGE(TAG, "OTA action id string size exceeded max length of buffer");
                return -1;
            }

            ota_update_completed = 1;
            BB_LOGI(TAG, "Reboot after successful OTA update");
        } else {
            BB_LOGI(TAG, "Normal reboot");
        }
    }

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        BB_LOGI(TAG, "Device contains factory Firmware\n");
    }

    nvs_close(temp_nv_handle);
    return 0;
}

int bytebeam_hal_destroy(bytebeam_client_t *bytebeam_client)
{
    esp_err_t err;

    err = esp_mqtt_client_destroy(bytebeam_client->client);

    if (err != ESP_OK) {
        return -1;
    }

    return 0;
}

int bytebeam_hal_start_mqtt(bytebeam_client_t *bytebeam_client)
{
    esp_err_t err;

    err = esp_mqtt_client_start(bytebeam_client->client);

    if (err != ESP_OK) {
        return -1;
    }

    if (ota_update_completed == 1) {
        ota_update_completed = 0;

        if ((bytebeam_publish_action_completed(bytebeam_client, ota_action_id_str)) != 0) {
            BB_LOGE(TAG, "Failed to publish OTA complete status");
        }
    }

    xTaskCreate(bytebeam_user_thread_entry, "Bytebeam User Thread", 4*1024, bytebeam_client, 2, NULL);
    xTaskCreate(bytebeam_mqtt_thread_entry, "Bytebeam MQTT Thread", 8*1024, NULL, 2, NULL);

    return 0;
}

int bytebeam_hal_stop_mqtt(bytebeam_client_t *bytebeam_client)
{
    esp_err_t err;

    err = esp_mqtt_client_stop(bytebeam_client->client);

    if (err != ESP_OK) {
        return -1;
    }

    return 0;
}

int bytebeam_hal_spiffs_mount()
{  
    esp_err_t err;

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    err = esp_vfs_spiffs_register(&conf);

    if (err != ESP_OK)
    {
        switch(err)
        {
            case ESP_FAIL:
                BB_LOGE(TAG, "Failed to mount or format SPIFFS");
                break;

            case ESP_ERR_NOT_FOUND:
                BB_LOGE(TAG, "Unable to find SPIFFS partition");
                break;

            default:
                BB_LOGE(TAG, "Failed to register SPIFFS partition (%s)", esp_err_to_name(err));
        }

        return -1;
    }

    return 0;
}

int bytebeam_hal_spiffs_unmount()
{
    esp_err_t err;

    err = esp_vfs_spiffs_unregister(NULL);

    if (err != ESP_OK) {
        return -1;
    }

    return 0;
}

int bytebeam_hal_fatfs_mount()
{
    esp_err_t err;

    const esp_vfs_fat_mount_config_t conf = {
            .max_files = 4,
            .format_if_mount_failed = false,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    err = esp_vfs_fat_spiflash_mount_ro("/spiflash", "storage", &conf);

    if (err != ESP_OK)
    {
        switch(err)
        {
            case ESP_FAIL:
                BB_LOGE(TAG, "Failed to mount FATFS");
                break;

            case ESP_ERR_NOT_FOUND:
                BB_LOGE(TAG, "Unable to find FATFS partition");
                break;

            default:
                BB_LOGE(TAG, "Failed to register FATFS (%s)", esp_err_to_name(err));
        }

        return -1;
    }

    return 0;
}

int bytebeam_hal_fatfs_unmount()
{
    esp_err_t err;

    err = esp_vfs_fat_spiflash_unmount_ro("/spiflash", "storage");

    if (err != ESP_OK) {
        return -1;
    }

    return 0;
}

unsigned long long bytebeam_hal_get_epoch_millis()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

bytebeam_reset_reason_t bytebeam_hal_get_reset_reason()
{
    bytebeam_reset_reason_t reboot_reason_id = esp_reset_reason();
    return reboot_reason_id;
}

long long bytebeam_hal_get_uptime_ms()
{
    long long uptime = esp_timer_get_time();
    uptime = uptime/1000;
    return uptime;
}