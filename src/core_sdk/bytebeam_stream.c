#include "cJSON.h"
#include "bytebeam_hal.h"
#include "bytebeam_action.h"
#include "bytebeam_stream.h"

static const char *TAG = "BYTEBEAM_STREAM";

bytebeam_err_t bytebeam_publish_to_stream(bytebeam_client_t *bytebeam_client, char *stream_name, char *payload)
{
    if (bytebeam_client == NULL || stream_name == NULL || payload == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

    int qos = 1;
    int msg_id = 0;
    char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = {0};

    int max_len = BYTEBEAM_MQTT_TOPIC_STR_LEN;
    int temp_var = snprintf(topic, max_len,  "/tenants/%s/devices/%s/events/%s/jsonarray",
            bytebeam_client->device_cfg.project_id,
            bytebeam_client->device_cfg.device_id,
            stream_name);

    if(temp_var >= max_len)
    {
        BB_LOGE(TAG, "Publish topic size exceeded buffer size");
        return BB_FAILURE;
    }

    BB_LOGI(TAG, "Topic is %s", topic);

    msg_id = bytebeam_hal_mqtt_publish(bytebeam_client->client, topic, payload, strlen(payload), qos);
    
    if (msg_id != -1) {
        BB_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id, payload);
        return BB_SUCCESS;
    } else {
        BB_LOGE(TAG, "Publish to %s stream Failed", stream_name);
        return BB_FAILURE;
    }
}

bytebeam_err_t bytebeam_publish_device_shadow(bytebeam_client_t *bytebeam_client)
{
    bytebeam_err_t ret_val = 0;
    bytebeam_reset_reason_t reboot_reason_id;

    cJSON *device_shadow_json_list = NULL;
    cJSON *device_shadow_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *device_status_json = NULL;
    cJSON *device_reset_reason_json = NULL;
    cJSON *device_uptime_json = NULL;
    cJSON *device_software_type_json = NULL;
    cJSON *device_software_version_json = NULL;
    cJSON *device_hardware_type_json = NULL;
    cJSON *device_hardware_version_json = NULL;

    char *string_json = NULL;

    device_shadow_json_list = cJSON_CreateArray();

    if(device_shadow_json_list == NULL)
    {
        BB_LOGE(TAG, "Json Init failed.");
        return -1;
    }

    device_shadow_json = cJSON_CreateObject();

    if(device_shadow_json == NULL)
    {
        BB_LOGE(TAG, "Json add failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    bytebeam_client->device_shadow.stream.milliseconds = bytebeam_hal_get_epoch_millis();

    if(bytebeam_client->device_shadow.stream.milliseconds == 0)
    {
        BB_LOGE(TAG, "failed to get epoch millis.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    timestamp_json = cJSON_CreateNumber(bytebeam_client->device_shadow.stream.milliseconds);

    if(timestamp_json == NULL)
    {
        BB_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

    bytebeam_client->device_shadow.stream.sequence++;
    sequence_json = cJSON_CreateNumber(bytebeam_client->device_shadow.stream.sequence);

    if(sequence_json == NULL)
    {
        BB_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "sequence", sequence_json);

    reboot_reason_id = bytebeam_hal_get_reset_reason();

    switch(reboot_reason_id) {
        case BB_RST_UNKNOWN   : bytebeam_client->device_shadow.stream.reboot_reason = "Unknown Reset";            break;
        case BB_RST_POWERON   : bytebeam_client->device_shadow.stream.reboot_reason = "Power On Reset";           break;
        case BB_RST_EXT       : bytebeam_client->device_shadow.stream.reboot_reason = "External Pin Reset";       break;
        case BB_RST_SW        : bytebeam_client->device_shadow.stream.reboot_reason = "Software Reset";           break;
        case BB_RST_PANIC     : bytebeam_client->device_shadow.stream.reboot_reason = "Hard Fault Reset";         break;
        case BB_RST_INT_WDT   : bytebeam_client->device_shadow.stream.reboot_reason = "Interrupt Watchdog Reset"; break;
        case BB_RST_TASK_WDT  : bytebeam_client->device_shadow.stream.reboot_reason = "Task Watchdog Reset";      break;
        case BB_RST_WDT       : bytebeam_client->device_shadow.stream.reboot_reason = "Other Watchdog Reset";     break;
        case BB_RST_DEEPSLEEP : bytebeam_client->device_shadow.stream.reboot_reason = "Exiting Deep Sleep Reset"; break;
        case BB_RST_BROWNOUT  : bytebeam_client->device_shadow.stream.reboot_reason = "Brownout Reset";           break;
        case BB_RST_SDIO      : bytebeam_client->device_shadow.stream.reboot_reason = "SDIO Reset";               break;

        default: bytebeam_client->device_shadow.stream.reboot_reason = "Unknown Reset Id";
    }

    device_reset_reason_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.reboot_reason);

    if(device_reset_reason_json == NULL)
    {
        BB_LOGE(TAG, "Json add device reboot reason failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Reset_Reason", device_reset_reason_json);

    bytebeam_client->device_shadow.stream.uptime = bytebeam_hal_get_uptime_ms();

    device_uptime_json = cJSON_CreateNumber(bytebeam_client->device_shadow.stream.uptime);

    if(device_uptime_json == NULL)
    {
        BB_LOGE(TAG, "Json add device uptime failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Uptime", device_uptime_json);

    // get the device shadow status from config menu
    bytebeam_client->device_shadow.stream.status = CONFIG_DEVICE_SHADOW_STATUS;

    device_status_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.status);

    if(device_status_json == NULL)
    {
        BB_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    // get the device shadow software type from config menu
    bytebeam_client->device_shadow.stream.software_type = CONFIG_DEVICE_SHADOW_SOFTWARE_TYPE;

    device_software_type_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.software_type);

    if(device_software_type_json == NULL)
    {
        BB_LOGE(TAG, "Json add device software type failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Software_Type", device_software_type_json);

    // get the device shadow software version from config menu
    bytebeam_client->device_shadow.stream.software_version = CONFIG_DEVICE_SHADOW_SOFTWARE_VERSION;

    device_software_version_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.software_version);

    if(device_software_version_json == NULL)
    {
        BB_LOGE(TAG, "Json add device software version failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Software_Version", device_software_version_json);

    // get the device shadow hardware type from config menu
    bytebeam_client->device_shadow.stream.hardware_type = CONFIG_DEVICE_SHADOW_HARDWARE_TYPE;

    device_hardware_type_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.hardware_type);

    if(device_hardware_type_json == NULL)
    {
        BB_LOGE(TAG, "Json add device hardware type failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Hardware_Type", device_hardware_type_json);

    // get the device shadow hardware version from config menu
    bytebeam_client->device_shadow.stream.hardware_version = CONFIG_DEVICE_SHADOW_HARDWARE_VERSION;

    device_hardware_version_json = cJSON_CreateString(bytebeam_client->device_shadow.stream.hardware_version);

    if(device_hardware_version_json == NULL)
    {
        BB_LOGE(TAG, "Json add device hardware version failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Hardware_Version", device_hardware_version_json);

    cJSON_AddItemToArray(device_shadow_json_list, device_shadow_json);

    string_json = cJSON_Print(device_shadow_json_list);

    if(string_json == NULL)
    {
        BB_LOGE(TAG, "Json string print failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    BB_LOGI(TAG, "\nStatus to send:\n%s\n", string_json);

    // publish the json to device shadow stream
    ret_val = bytebeam_publish_to_stream(bytebeam_client, "device_shadow", string_json);

    cJSON_Delete(device_shadow_json_list);
    cJSON_free(string_json);

    return ret_val;
}

void bytebeam_user_thread_entry(void *pv)
{
    bytebeam_err_t err_code;
    bytebeam_client_t *bytebeam_client = (bytebeam_client_t*) pv;

    while(1)
    {
        BB_LOGI(TAG, "Device Shadow Message.\n");
        
        err_code = bytebeam_publish_device_shadow(bytebeam_client);

        if(err_code != BB_SUCCESS)
        {
            BB_LOGE(TAG, "Failed to push Device Shadow Seq : %llu\n", bytebeam_client->device_shadow.stream.sequence);
        }

        vTaskDelay(CONFIG_DEVICE_SHADOW_PUSH_INTERVAL * 1000 / portTICK_PERIOD_MS);
    }
}

void bytebeam_mqtt_thread_entry(void *pv)
{
    bytebeam_err_t err_code;
    bytebeam_client_t *bytebeam_client = (bytebeam_client_t*) pv;

    while(1)
    {
        BB_LOGI(TAG, "Bytebeam MQTT Thread\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}