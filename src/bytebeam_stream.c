#include "cJSON.h"
#include "bytebeam_esp_hal.h"
#include "bytebeam_action.h"
#include "bytebeam_stream.h"

static const char *TAG = "BYTEBEAM_STREAM";

int bytebeam_publish_device_heartbeat(bytebeam_client_t *bytebeam_client)
{
    static uint64_t sequence = 0;
    unsigned long long milliseconds = 0;

    int ret_val = 0;
    const char* reboot_reason_str = "";
    long long uptime = 0;

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

    milliseconds = bytebeam_hal_get_epoch_millis();

    if(milliseconds == 0)
    {
        BB_LOGE(TAG, "failed to get epoch millis.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if(timestamp_json == NULL)
    {
        BB_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if(sequence_json == NULL)
    {
        BB_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "sequence", sequence_json);

    bytebeam_reset_reason_t reboot_reason_id = bytebeam_hal_get_reset_reason();

    switch(reboot_reason_id) {
        case BB_RST_UNKNOWN   : reboot_reason_str = "Unknown Reset";            break;
        case BB_RST_POWERON   : reboot_reason_str = "Power On Reset";           break;
        case BB_RST_EXT       : reboot_reason_str = "External Pin Reset";       break;
        case BB_RST_SW        : reboot_reason_str = "Software Reset";           break;
        case BB_RST_PANIC     : reboot_reason_str = "Hard Fault Reset";         break;
        case BB_RST_INT_WDT   : reboot_reason_str = "Interrupt Watchdog Reset"; break;
        case BB_RST_TASK_WDT  : reboot_reason_str = "Task Watchdog Reset";      break;
        case BB_RST_WDT       : reboot_reason_str = "Other Watchdog Reset";     break;
        case BB_RST_DEEPSLEEP : reboot_reason_str = "Exiting Deep Sleep Reset"; break;
        case BB_RST_BROWNOUT  : reboot_reason_str = "Brownout Reset";           break;
        case BB_RST_SDIO      : reboot_reason_str = "SDIO Reset";               break;

        default: reboot_reason_str = "Unknown Reset Id";
    }

    device_reset_reason_json = cJSON_CreateString(reboot_reason_str);

    if(device_reset_reason_json == NULL)
    {
        BB_LOGE(TAG, "Json add device reboot reason failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Reset_Reason", device_reset_reason_json);

    uptime = bytebeam_hal_get_uptime_ms();

    device_uptime_json = cJSON_CreateNumber(uptime);

    if(device_uptime_json == NULL)
    {
        BB_LOGE(TAG, "Json add device uptime failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Uptime", device_uptime_json);

    // if status is not provided append the dummy one showing device activity
    if(bytebeam_client->device_info.status == NULL) {
        bytebeam_client->device_info.status = "Device is Active!";
    }

    device_status_json = cJSON_CreateString(bytebeam_client->device_info.status);

    if(device_status_json == NULL)
    {
        BB_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    // if software type is provided append it else leave
    if(bytebeam_client->device_info.software_type != NULL) {
        device_software_type_json = cJSON_CreateString(bytebeam_client->device_info.software_type);

        if(device_software_type_json == NULL)
        {
            BB_LOGE(TAG, "Json add device software type failed.");
            cJSON_Delete(device_shadow_json_list);
            return -1;
        }

        cJSON_AddItemToObject(device_shadow_json, "Software_Type", device_software_type_json);
    }

    // if software version is provided append it else leave
    if(bytebeam_client->device_info.software_version != NULL) {
         device_software_version_json = cJSON_CreateString(bytebeam_client->device_info.software_version);

        if(device_software_version_json == NULL)
        {
            BB_LOGE(TAG, "Json add device software version failed.");
            cJSON_Delete(device_shadow_json_list);
            return -1;
        }

        cJSON_AddItemToObject(device_shadow_json, "Software_Version", device_software_version_json);
    }

    // if hardware type is provided append it else leave
    if(bytebeam_client->device_info.hardware_type != NULL) {

        device_hardware_type_json = cJSON_CreateString(bytebeam_client->device_info.hardware_type);

        if(device_hardware_type_json == NULL)
        {
            BB_LOGE(TAG, "Json add device hardware type failed.");
            cJSON_Delete(device_shadow_json_list);
            return -1;
        }

        cJSON_AddItemToObject(device_shadow_json, "Hardware_Type", device_hardware_type_json);
    }

    // if hardware version is provided append it else leave
    if(bytebeam_client->device_info.hardware_version != NULL) {
        device_hardware_version_json = cJSON_CreateString(bytebeam_client->device_info.hardware_version);

        if(device_hardware_version_json == NULL)
        {
            BB_LOGE(TAG, "Json add device hardware version failed.");
            cJSON_Delete(device_shadow_json_list);
            return -1;
        }

        cJSON_AddItemToObject(device_shadow_json, "Hardware_Version", device_hardware_version_json);
    }

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
