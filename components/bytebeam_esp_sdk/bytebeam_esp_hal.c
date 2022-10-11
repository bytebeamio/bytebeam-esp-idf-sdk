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

static int ota_img_data_len = 0;
static int ota_update_completed = 0;
static char ota_action_id_str[15];

static bytebeam_client_handle_t mqtt_client_handle;
static bytebeam_device_config_t temp_device_config;

static const char *TAG_BYTE_BEAM_ESP_HAL = "BYTEBEAM_SDK";

int bytebeam_hal_mqtt_subscribe(void *client, char *topic, int qos)
{
    return esp_mqtt_client_subscribe(client, topic, qos);
}

int bytebeam_hal_mqtt_publish(void *client, char *topic, const char *message, int length, int qos)
{
    return esp_mqtt_client_publish(client, topic, (const char *)message, length, 1, 1);
}

int bytebeam_hal_restart(void)
{
    esp_restart();
    return 0;
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int loop_var = 0;
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
            if (loop_var == 100) {
                if ((publish_action_status(temp_device_config, ota_action_id, update_progress_percent, mqtt_client_handle, "Complete", "Success")) != 0) {
                    ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to publish OTA progress status");
                }
            } else {
                if ((publish_action_status(temp_device_config, ota_action_id, update_progress_percent, mqtt_client_handle, "Progress", "Success")) != 0) {
                    ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to publish OTA progress status");
                }
            }

            loop_var = loop_var + 5;
        }

        if (loop_var == 105) {
            loop_var = 0;
            update_progress_percent = 0;
            downloaded_data_len = 0;
        }

        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_ON_FINISH");
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "HTTP_EVENT_DISCONNECTED");
        break;
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

int bytebeam_hal_ota(bytebeam_client_t *bb_obj, char *ota_url)
{
    bytebeam_device_config_t *device_cfg = &bb_obj->device_cfg;
    mqtt_client_handle = bb_obj->client;
    temp_device_config = bb_obj->device_cfg;

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

    if ((esp_https_ota(&config)) != ESP_OK) {
        return -1;
    }

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
    ESP_LOGD(TAG_BYTE_BEAM_ESP_HAL, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);

    esp_mqtt_event_handle_t event = event_data;
    bytebeam_client_handle_t client = event->client;
    bytebeam_client_t *bb_obj = handler_args;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_CONNECTED");
        msg_id = bytebeam_subscribe_to_actions(bb_obj->device_cfg, client);

        if (msg_id != -1) {
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT SUBSCRIBED!! Msg ID:%d", msg_id);
        } else {
            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT SUBSCRIBE FAILED");
        }

        bb_obj->connection_status = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "MQTT_EVENT_DISCONNECTED");
        bb_obj->connection_status = 0;
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

        bytebeam_handle_actions(event->data, event->client, bb_obj);
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

int bytebeam_hal_init(bytebeam_client_t *bb_obj)
{
    int32_t update_flag;
    nvs_handle_t temp_nv_handle;
    esp_err_t err;

    ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    bb_obj->client = esp_mqtt_client_init(&bb_obj->mqtt_cfg);

    if (bb_obj->client == NULL) {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "MQTT Client initialization failed");
        return -1;
    }

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    err = esp_mqtt_client_register_event(bb_obj->client, ESP_EVENT_ANY_ID, mqtt_event_handler, bb_obj);

    if (err != ESP_OK) {
        ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to register MQTT event callback");
        return -1;
    }

    bytebeam_init_action_handler_array(bb_obj->action_funcs);

    nvs_open("test_storage", NVS_READWRITE, &temp_nv_handle);
    err = nvs_get_i32(temp_nv_handle, "update_flag", &update_flag);

    if (err == ESP_OK) {
        if (update_flag == 1) {
            int32_t ota_action_id_val;
            update_flag = 0;

            nvs_set_i32(temp_nv_handle, "update_flag", update_flag);
            nvs_commit(temp_nv_handle);

            ESP_LOGI(TAG_BYTE_BEAM_ESP_HAL, "Reboot after successful OTA update");

            nvs_get_i32(temp_nv_handle, "action_id_val", &ota_action_id_val);
            sprintf(ota_action_id_str, "%d", ota_action_id_val);

            ota_update_completed = 1;
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

int bytebeam_hal_start_mqtt(bytebeam_client_t *bb_obj)
{
    esp_err_t err;

    err = esp_mqtt_client_start(bb_obj->client);

    if (err != ESP_OK) {
        return -1;
    }

    if (ota_update_completed == 1) {
        ota_update_completed = 0;

        if ((bytebeam_publish_action_completed(bb_obj, ota_action_id_str)) != 0) {
            ESP_LOGE(TAG_BYTE_BEAM_ESP_HAL, "Failed to publish OTA complete status");
        }
    }

    return 0;
}