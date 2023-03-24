#include <stdio.h>
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"
#include "math.h"
#include "string.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"

/*This macro is used to specify the maximum length of OTA action id string including NULL Character*/
#define OTA_ACTION_ID_STR_LEN 20

static int ota_img_data_len = 0;
static int ota_update_completed = 0;
static char ota_action_id_str[OTA_ACTION_ID_STR_LEN];

static bytebeam_client_handle_t mqtt_client_handle;
static bytebeam_device_config_t temp_device_config;

static const char *TAG_BYTE_BEAM_ESP_HAL = "BYTEBEAM_SDK";

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

    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ERROR");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ON_CONNECTED");
        break;

    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_HEADER_SENT");
        break;

    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        downloaded_data_len = downloaded_data_len + evt->data_len;
        update_progress_percent = (((float)downloaded_data_len / (float)ota_img_data_len) * 100.00);

        if (update_progress_percent == loop_var) {
            ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "update_progress_percent : %d", update_progress_percent);
            ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "ota_action_id : %s", ota_action_id);

            // publish the OTA progress
            if(publish_action_status(temp_device_config, ota_action_id, update_progress_percent, mqtt_client_handle, "Progress", "Success") != 0) {
                ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to publish OTA progress status");
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
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ON_FINISH");
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_DISCONNECTED");
        break;
	
#ifdef BYTEBEAM_ESP_IDF_VERSION_5_0
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_REDIRECT");
        break;
#endif
    }

    return ESP_OK;
}

esp_err_t _test_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ERROR");
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
    bytebeam_device_config_t *device_cfg = &(bytebeam_client->device_cfg);
    mqtt_client_handle = bytebeam_client->client;
    temp_device_config = bytebeam_client->device_cfg;

    esp_http_client_config_t config = {
        .url = ota_url,
        .cert_pem = (char *)device_cfg->ca_cert_pem,
        .client_cert_pem = (char *)device_cfg->client_cert_pem,
        .client_key_pem = (char *)device_cfg->client_key_pem,
        .event_handler = _http_event_handler,
    };

    esp_http_client_config_t test_config = {
        .url = ota_url,
        .cert_pem = (char *)device_cfg->ca_cert_pem,
        .client_cert_pem = (char *)device_cfg->client_cert_pem,
        .client_key_pem = (char *)device_cfg->client_key_pem,
        .event_handler = _test_event_handler,
    };

#ifdef BYTEBEAM_ESP_IDF_VERSION_5_0
	esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
#endif

    ota_img_data_len = 0;

    esp_http_client_handle_t client = esp_http_client_init(&test_config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "content_length = %d", ota_img_data_len);
    } else {
        return -1;
    }

    esp_http_client_cleanup(client);
    ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "The URL is:%s", config.url);

#ifdef BYTEBEAM_ESP_IDF_VERSION_5_0
    if ((esp_https_ota(&ota_config)) != ESP_OK) {
        return -1;
    }
#endif

#ifdef BYTEBEAM_ESP_IDF_VERSION_4_4_3
    if ((esp_https_ota(&config)) != ESP_OK) {
        return -1;
    }
#endif

    return 0;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Last error %s: 0x%x", message, error_code);
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
    ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);

    esp_mqtt_event_handle_t event = event_data;
    bytebeam_client_handle_t client = event->client;
    bytebeam_client_t *bytebeam_client = handler_args;
    int msg_id, ret_val;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_CONNECTED");
        msg_id = bytebeam_subscribe_to_actions(bytebeam_client->device_cfg, client);

        if (msg_id != -1) {
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT SUBSCRIBED!! Msg ID:%d", msg_id);
        } else {
            ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "MQTT SUBSCRIBE FAILED");
        }

        bytebeam_client->connection_status = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_DISCONNECTED");
        bytebeam_client->connection_status = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "DATA=%.*s\r\n", event->data_len, event->data);

        ret_val = bytebeam_handle_actions(event->data, event->client, bytebeam_client);

        if (ret_val != 0) {
            ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "BYTEBEAM HANDLE ACTIONS FAILED");
        } else {
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "BYTEBEAM HANDLE ACTIONS SUCCESS!!");
        }

        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_ERROR");

        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }

        break;

    default:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Other event id:%d", event->event_id);
        break;
    }
}

int bytebeam_hal_init(bytebeam_client_t *bytebeam_client)
{
    int32_t update_flag;
    nvs_handle_t temp_nv_handle;
    esp_err_t err;

    ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "[APP] Free memory: %d bytes", (int)esp_get_free_heap_size());

    bytebeam_client->client = esp_mqtt_client_init(&bytebeam_client->mqtt_cfg);

    if (bytebeam_client->client == NULL) {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "MQTT Client initialization failed");
        return -1;
    }

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    err = esp_mqtt_client_register_event(bytebeam_client->client, ESP_EVENT_ANY_ID, mqtt_event_handler, bytebeam_client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to register MQTT event callback");
        return -1;
    }

    err = nvs_flash_init();

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "NVS flash init failed.");
        return -1;
    }

    err = nvs_open("test_storage", NVS_READWRITE, &temp_nv_handle);

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to open NVS Storage");
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
                ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to reset the OTA update flag in NVS");
                return -1;
            }

            temp_err = nvs_commit(temp_nv_handle);

            if (temp_err != ESP_OK) 
            {
                ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to commit the OTA update flag in NVS");
                return -1;
            }

            /* Get the OTA action id from NVS */
            temp_err = nvs_get_i32(temp_nv_handle, "action_id_val", &ota_action_id_val);

            if (temp_err != ESP_OK) 
            {
                ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to retreieve the OTA action id from NVS");
                return -1;
            }

            int max_len = OTA_ACTION_ID_STR_LEN;
            int temp_var = snprintf(ota_action_id_str, max_len, "%d", (int)ota_action_id_val);

            if(temp_var >= max_len)
            {
                ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "OTA action id string size exceeded max length of buffer");
                return -1;
            }

            ota_update_completed = 1;
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Reboot after successful OTA update");
        } else {
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Normal reboot");
        }
    }

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Device contains factory Firmware\n");
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
            ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to publish OTA complete status");
        }
    }

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