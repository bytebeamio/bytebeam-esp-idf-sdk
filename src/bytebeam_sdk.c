#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_log.h"
#include "cJSON.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_sntp.h"
#include "esp_timer.h"
#include "esp_idf_version.h"

char *ota_action_id = "";
char ota_error_str[BYTEBEAM_OTA_ERROR_STR_LEN] = "";

static int function_handler_index = 0;
static char bytebeam_last_known_action_id[BYTEBEAM_ACTION_ID_STR_LEN] = { 0 };

static const char *TAG = "BYTEBEAM_SDK";

int bytebeam_subscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client)
{
    int msg_id;
    int qos = 1;
    char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

    int max_len = BYTEBEAM_MQTT_TOPIC_STR_LEN;
    int temp_var = snprintf(topic, max_len, "/tenants/%s/devices/%s/actions", device_cfg.project_id, device_cfg.device_id);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "subscribe topic size exceeded buffer size");
        return -1;
    }

    ESP_LOGD(TAG, "Subscribe Topic is %s", topic);

    msg_id = bytebeam_hal_mqtt_subscribe(client, topic, qos);

    return msg_id;
}

int bytebeam_unsubscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client)
{
    int msg_id;
    char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = { 0 };

    int max_len = BYTEBEAM_MQTT_TOPIC_STR_LEN;
    int temp_var = snprintf(topic, max_len, "/tenants/%s/devices/%s/actions", device_cfg.project_id, device_cfg.device_id);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "unsubscribe topic size exceeded buffer size");
        return -1;
    }

    ESP_LOGD(TAG, "Unsubscribe Topic is %s", topic);

    /* Commenting call to hal unsubscribe api as cloud seems to be not supporting unsubscribe feature, will test it
     * once cloud supports unsubscribe feature, most probably it will work.
     */

    // msg_id = bytebeam_hal_mqtt_unsubscribe(client, topic);
    msg_id = 1234;
    ESP_LOGI(TAG, "We will add the unsubscribe to actions feature soon");

    return msg_id;
}

int bytebeam_handle_actions(char *action_received, bytebeam_client_handle_t client, bytebeam_client_t *bytebeam_client)
{
    cJSON *root = NULL;
    cJSON *name = NULL;
    cJSON *payload = NULL;
    cJSON *action_id_obj = NULL;

    int action_iterator = 0;
    char action_id[BYTEBEAM_ACTION_ID_STR_LEN] = { 0 };

    root = cJSON_Parse(action_received);

    if (root == NULL) {
        ESP_LOGE(TAG, "ERROR in parsing the JSON\n");

        return -1;
    }

    name = cJSON_GetObjectItem(root, "name");

    if (!(cJSON_IsString(name) && (name->valuestring != NULL))) {
        ESP_LOGE(TAG, "Error parsing action name\n");

        cJSON_Delete(root);
        return -1;
    }

    ESP_LOGI(TAG, "Checking name \"%s\"\n", name->valuestring);

    action_id_obj = cJSON_GetObjectItem(root, "id");

    if (cJSON_IsString(action_id_obj) && (action_id_obj->valuestring != NULL)) {
        ESP_LOGI(TAG, "Checking version \"%s\"\n", action_id_obj->valuestring);
    } else {
        ESP_LOGE(TAG, "Error parsing action id");

        cJSON_Delete(root);
        return -1;
    }

    int max_len = BYTEBEAM_ACTION_ID_STR_LEN;
    int temp_var = snprintf(action_id, max_len, "%s", action_id_obj->valuestring);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "Action Id length exceeded buffer size");

        cJSON_Delete(root);
        return -1;
    }

    int action_id_val = atoi(action_id);
    int last_known_action_id_val = atoi(bytebeam_last_known_action_id);

    ESP_LOGD(TAG, "action_id_val : %d, last_known_action_id_val : %d\n",action_id_val, last_known_action_id_val);

    // just ignore the previous actions if triggered again
    if (action_id_val <= last_known_action_id_val) {
        ESP_LOGE(TAG, "Ignoring %s Action\n", name->valuestring);
        return 0;
    }

    payload = cJSON_GetObjectItem(root, "payload");

    if (cJSON_IsString(payload) && (payload->valuestring != NULL)) {
        ESP_LOGI(TAG, "Checking payload \"%s\"\n", payload->valuestring);

        while (bytebeam_client->action_funcs[action_iterator].name) {
            if (!strcmp(bytebeam_client->action_funcs[action_iterator].name, name->valuestring)) {
                bytebeam_client->action_funcs[action_iterator].func(bytebeam_client, payload->valuestring, action_id);
                break;
            }

            action_iterator++;
        }

        if (bytebeam_client->action_funcs[action_iterator].name == NULL) {
            ESP_LOGI(TAG, "Invalid action:%s\n", name->valuestring);

            // publish action failed response indicating unregistered action
            bytebeam_publish_action_status(bytebeam_client, action_id, 0, "Failed", "Unregistered Action");
        }

        // update the last known action id
        strcpy(bytebeam_last_known_action_id, action_id);
    } else {
        ESP_LOGE(TAG, "Error fetching payload");

        cJSON_Delete(root);
        return -1;
    }

    /*  Deleting json object from memory after calling the respective action handlder so make sure that they will not
     *  gonna use the json object beyond the scope of the handler as this pointer we will be dangling.
     */
    cJSON_Delete(root);

    return 0;
}

int bytebeam_publish_device_heartbeat(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;
    struct timeval te;
    long long milliseconds = 0;
    static uint64_t sequence = 0;
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

    esp_reset_reason_t reboot_reason_id = esp_reset_reason();

    switch(reboot_reason_id) {
        case ESP_RST_UNKNOWN   : reboot_reason_str = "Unknown Reset";            break;
        case ESP_RST_POWERON   : reboot_reason_str = "Power On Reset";           break;
        case ESP_RST_EXT       : reboot_reason_str = "External Pin Reset";       break;
        case ESP_RST_SW        : reboot_reason_str = "Software Reset";           break;
        case ESP_RST_PANIC     : reboot_reason_str = "Hard Fault Reset";         break;
        case ESP_RST_INT_WDT   : reboot_reason_str = "Interrupt Watchdog Reset"; break;
        case ESP_RST_TASK_WDT  : reboot_reason_str = "Task Watchdog Reset";      break;
        case ESP_RST_WDT       : reboot_reason_str = "Other Watchdog Reset";     break;
        case ESP_RST_DEEPSLEEP : reboot_reason_str = "Exiting Deep Sleep Reset"; break;
        case ESP_RST_BROWNOUT  : reboot_reason_str = "Brownout Reset";           break;
        case ESP_RST_SDIO      : reboot_reason_str = "SDIO Reset";               break;

        default: reboot_reason_str = "Unknown Reset Id";
    }

    device_reset_reason_json = cJSON_CreateString(reboot_reason_str);

    if(device_reset_reason_json == NULL)
    {
        ESP_LOGE(TAG, "Json add device reboot reason failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Reset_Reason", device_reset_reason_json);

    // get the device up time
    uptime = esp_timer_get_time();

    // change to millis interval
    uptime = uptime/1000;

    device_uptime_json = cJSON_CreateNumber(uptime);

    if(device_uptime_json == NULL)
    {
        ESP_LOGE(TAG, "Json add device uptime failed.");
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
        ESP_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    // if software type is provided append it else leave
    if(bytebeam_client->device_info.software_type != NULL) {
        device_software_type_json = cJSON_CreateString(bytebeam_client->device_info.software_type);

        if(device_software_type_json == NULL)
        {
            ESP_LOGE(TAG, "Json add device software type failed.");
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
            ESP_LOGE(TAG, "Json add device software version failed.");
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
            ESP_LOGE(TAG, "Json add device hardware type failed.");
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
            ESP_LOGE(TAG, "Json add device hardware version failed.");
            cJSON_Delete(device_shadow_json_list);
            return -1;
        }

        cJSON_AddItemToObject(device_shadow_json, "Hardware_Version", device_hardware_version_json);
    }

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
    ret_val = bytebeam_publish_to_stream(bytebeam_client, "device_shadow", string_json);

    cJSON_Delete(device_shadow_json_list);
    cJSON_free(string_json);

    return ret_val;
}

bytebeam_err_t bytebeam_publish_action_completed(bytebeam_client_t *bytebeam_client, char *action_id)
{
    int ret_val = 0;

    ret_val = bytebeam_publish_action_status(bytebeam_client, action_id, 100, "Completed", "");

    if (ret_val != 0) {
        return BB_FAILURE;
    } else {
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_publish_action_failed(bytebeam_client_t *bytebeam_client, char *action_id)
{
    int ret_val = 0;

    ret_val = bytebeam_publish_action_status(bytebeam_client, action_id, 0, "Failed", "Action failed");

    if (ret_val != 0) {
        return BB_FAILURE;
    } else {
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_publish_action_progress(bytebeam_client_t *bytebeam_client, char *action_id, int progress_percentage)
{
    int ret_val = 0;

    ret_val = bytebeam_publish_action_status(bytebeam_client, action_id, progress_percentage, "Progress", "");

    if (ret_val != 0) {
        return BB_FAILURE;
    } else {
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_publish_action_status(bytebeam_client_t *bytebeam_client, char *action_id, int percentage, char *status, char *error_message)
{
    static uint64_t sequence = 0;
    cJSON *action_status_json_list = NULL;
    cJSON *action_status_json = NULL;
    cJSON *percentage_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *device_status_json = NULL;
    cJSON *action_id_json = NULL;
    cJSON *action_errors_json = NULL;
    cJSON *seq_json = NULL;
    char *string_json = NULL;
    const char *ota_states[1] = { "" };

    int qos = 1;
    int msg_id = 0;
    char topic[BYTEBEAM_MQTT_TOPIC_STR_LEN] = {0};

    action_status_json_list = cJSON_CreateArray();

    if (action_status_json_list == NULL) {
        ESP_LOGE(TAG, "Json Init failed.");

        return BB_FAILURE;
    }

    action_status_json = cJSON_CreateObject();

    if (action_status_json == NULL) {
        ESP_LOGE(TAG, "Json add failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if (timestamp_json == NULL) {
        ESP_LOGE(TAG, "Json add time stamp failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "timestamp", timestamp_json);

    sequence++;
    seq_json = cJSON_CreateNumber(sequence);

    if (seq_json == NULL) {
        ESP_LOGE(TAG, "Json add seq id failed.");
     
        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "sequence", seq_json);

    device_status_json = cJSON_CreateString(status);

    if (device_status_json == NULL) {
        ESP_LOGE(TAG, "Json add device status failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "state", device_status_json);

    ota_states[0] = error_message;
    action_errors_json = cJSON_CreateStringArray(ota_states, 1);

    if (action_errors_json == NULL) {
        ESP_LOGE(TAG, "Json add action errors failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "errors", action_errors_json);

    action_id_json = cJSON_CreateString(action_id);

    if (action_id_json == NULL) {
        ESP_LOGE(TAG, "Json add action_id failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "id", action_id_json);

    percentage_json = cJSON_CreateNumber(percentage);

    if (percentage_json == NULL) {
        ESP_LOGE(TAG, "Json add progress percentage failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(action_status_json, "progress", percentage_json);

    cJSON_AddItemToArray(action_status_json_list, action_status_json);

    string_json = cJSON_Print(action_status_json_list);

    if(string_json == NULL)
    {
        ESP_LOGE(TAG, "Json string print failed.");

        cJSON_Delete(action_status_json_list);
        return BB_FAILURE;
    } 

    ESP_LOGD(TAG, "\nTrying to print:\n%s\n", string_json);

    int max_len = BYTEBEAM_MQTT_TOPIC_STR_LEN;
    int temp_var = snprintf(topic, max_len, "/tenants/%s/devices/%s/action/status", bytebeam_client->device_cfg.project_id, bytebeam_client->device_cfg.device_id);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "action status topic size exceeded topic buffer size");

        cJSON_Delete(action_status_json_list);
        cJSON_free(string_json);
        return BB_FAILURE;
    }

    ESP_LOGI(TAG, "\nTopic is %s\n", topic);

    msg_id = bytebeam_hal_mqtt_publish(bytebeam_client->client, topic, string_json, strlen(string_json), qos);

    if (msg_id != -1) {
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id, string_json);
    } else {
        ESP_LOGE(TAG, "Publish Failed.");

        cJSON_Delete(action_status_json_list);
        cJSON_free(string_json);
        return BB_FAILURE;
    }

    cJSON_Delete(action_status_json_list);
    cJSON_free(string_json);

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_publish_to_stream(bytebeam_client_t *bytebeam_client, char *stream_name, char *payload)
{
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
        ESP_LOGE(TAG, "Publish topic size exceeded buffer size");
        return BB_FAILURE;
    }

    ESP_LOGI(TAG, "Topic is %s", topic);

    msg_id = bytebeam_hal_mqtt_publish(bytebeam_client->client, topic, payload, strlen(payload), qos);
    
    if (msg_id != -1) {
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id, payload);
        return BB_SUCCESS;
    } else {
        ESP_LOGE(TAG, "Publish to %s stream Failed", stream_name);
        return BB_FAILURE;
    }
}

int parse_ota_json(char *payload_string, char *url_string_return)
{   
    cJSON *pl_json = NULL;
    const cJSON *url = NULL;
    const cJSON *version = NULL;

    pl_json = cJSON_Parse(payload_string);

    if (pl_json == NULL) {
        ESP_LOGE(TAG, "ERROR in parsing the OTA JSON\n");

        return -1;
    }

    url = cJSON_GetObjectItem(pl_json, "url");

    if (cJSON_IsString(url) && (url->valuestring != NULL)) {
        ESP_LOGI(TAG, "Checking url \"%s\"\n", url->valuestring);
    } else {
        ESP_LOGE(TAG, "URL parsing failed");

        cJSON_Delete(pl_json);
        return -1;
    }

    version = cJSON_GetObjectItem(pl_json, "version");

    if (cJSON_IsString(version) && (version->valuestring != NULL)) {
        ESP_LOGI(TAG, "Checking version \"%s\"\n", version->valuestring);
    } else {
        ESP_LOGE(TAG, "FW version parsing failed");

        cJSON_Delete(pl_json);
        return -1;
    }

    int max_len = BYTEBAM_OTA_URL_STR_LEN;
    int temp_var = snprintf(url_string_return, max_len, "%s", url->valuestring);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "FW update URL exceeded buffer size");

        cJSON_Delete(pl_json);
        return -1;
    }

    ESP_LOGI(TAG, "The constructed URL is: %s", url_string_return);

    cJSON_Delete(pl_json);
    
    return 0;
}

int perform_ota(bytebeam_client_t *bytebeam_client, char *action_id, char *ota_url)
{
    // test_device_config = bytebeam_client->device_cfg;
    ESP_LOGI(TAG, "Starting OTA.....");

    if ((bytebeam_hal_ota(bytebeam_client, ota_url)) != -1) {
        esp_err_t err;
        nvs_handle_t nvs_handle;
        int32_t update_flag = 1;
        int32_t action_id_val = (int32_t)(atoi(ota_action_id));

        err = nvs_flash_init();

        if(err != ESP_OK)
        {
            ESP_LOGE(TAG, "NVS flash init failed.");
            return -1;
        }

        err = nvs_open("test_storage", NVS_READWRITE, &nvs_handle);

        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to open NVS Storage");
            return -1;
        }

        err = nvs_set_i32(nvs_handle, "update_flag", update_flag);

        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to set the OTA update flag in NVS");
            return -1;
        }

        err = nvs_commit(nvs_handle);

        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to commit the OTA update flag in NVS");
            return -1;
        }

        nvs_set_i32(nvs_handle, "action_id_val", action_id_val);

        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to set the OTA action id in NVS");
            return -1;
        }

        err = nvs_commit(nvs_handle);

        if (err != ESP_OK) 
        {
            ESP_LOGE(TAG, "Failed to commit the OTA action id in NVS");
            return -1;
        }

        nvs_close(nvs_handle);
        bytebeam_hal_restart();
    } else {
        ESP_LOGE(TAG, "Firmware Upgrade Failed");

        if ((bytebeam_publish_action_status(bytebeam_client, action_id, 0, "Failed", ota_error_str)) != 0) {
            ESP_LOGE(TAG, "Failed to publish negative response for Firmware upgrade failure");
        }

        // clear the OTA error
        memset(ota_error_str, 0x00, sizeof(ota_error_str));

        return -1;
    }

    return 0;
}

bytebeam_err_t handle_ota(bytebeam_client_t *bytebeam_client, char *payload_string, char *action_id)
{
    char constructed_url[BYTEBAM_OTA_URL_STR_LEN] = { 0 };

    if ((parse_ota_json(payload_string, constructed_url)) == -1) {
        ESP_LOGE(TAG, "Firmware upgrade failed due to error in parsing OTA JSON");
        return BB_FAILURE;
    }

    ota_action_id = action_id;

    if ((perform_ota(bytebeam_client, action_id, constructed_url)) == -1) {
        return BB_FAILURE;
    }

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_add_action_handler(bytebeam_client_t *bytebeam_client, int (*func_ptr)(bytebeam_client_t *, char *, char *), char *func_name)
{
    if (function_handler_index >= BYTEBEAM_NUMBER_OF_ACTIONS) {
        ESP_LOGE(TAG, "Creation of new action handler failed");
        return BB_FAILURE;
    }

    int action_iterator = 0;

    // checking for duplicates in the array, if there log the info about it and return 
    for (action_iterator = 0; action_iterator < function_handler_index; action_iterator++) {
        if (!strcmp(bytebeam_client->action_funcs[action_iterator].name, func_name)) {
            ESP_LOGE(TAG, "action : %s is already there, update the action instead\n", func_name);
            return BB_FAILURE;
        }
    }

    bytebeam_client->action_funcs[function_handler_index].func = func_ptr;
    bytebeam_client->action_funcs[function_handler_index].name = func_name;

    function_handler_index = function_handler_index + 1;

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_remove_action_handler(bytebeam_client_t *bytebeam_client, char *func_name)
{
    int action_iterator = 0;
    int target_action_index = -1;

    for (action_iterator = 0; action_iterator < function_handler_index; action_iterator++) {
        if (!strcmp(bytebeam_client->action_funcs[action_iterator].name, func_name)) {
            target_action_index = action_iterator;
        }
    }

    if (target_action_index == -1) {
        ESP_LOGE(TAG, "action : %s not found \n", func_name);
        return BB_FAILURE;
    } else {
        for(action_iterator = target_action_index; action_iterator < function_handler_index - 1; action_iterator++) {
            bytebeam_client->action_funcs[action_iterator].func = bytebeam_client->action_funcs[action_iterator+1].func;
            bytebeam_client->action_funcs[action_iterator].name = bytebeam_client->action_funcs[action_iterator+1].name;
        }

        function_handler_index = function_handler_index - 1;
        bytebeam_client->action_funcs[function_handler_index].func = NULL;
        bytebeam_client->action_funcs[function_handler_index].name = NULL;
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_update_action_handler(bytebeam_client_t *bytebeam_client, int (*new_func_ptr)(bytebeam_client_t *, char *, char *), char *func_name)
{
    int action_iterator = 0;
    int target_action_index = -1;

    for (action_iterator = 0; action_iterator < function_handler_index; action_iterator++) {
        if (!strcmp(bytebeam_client->action_funcs[action_iterator].name, func_name)) {
            target_action_index = action_iterator;
        }
    }

    if (target_action_index == -1) {
        ESP_LOGE(TAG, "action : %s not found \n", func_name);
        return BB_FAILURE;
    } else {
        bytebeam_client->action_funcs[target_action_index].func = new_func_ptr;
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_is_action_handler_there(bytebeam_client_t *bytebeam_client, char *func_name)
{
    int action_iterator = 0;
    int target_action_index = -1;

    for (action_iterator = 0; action_iterator < function_handler_index; action_iterator++) {
        if (!strcmp(bytebeam_client->action_funcs[action_iterator].name, func_name)) {
            target_action_index = action_iterator;
        }
    }

    if (target_action_index == -1) {
        ESP_LOGE(TAG, "action : %s not found \n", func_name);
        return BB_FAILURE;
    } else {
        ESP_LOGI(TAG, "action : %s found at index %d\n", func_name, target_action_index);
        return BB_SUCCESS;
    }
}

void bytebeam_print_action_handler_array(bytebeam_client_t *bytebeam_client)
{
    int action_iterator = 0;

    ESP_LOGI(TAG, "[");
    for (action_iterator = 0; action_iterator < BYTEBEAM_NUMBER_OF_ACTIONS; action_iterator++) {
        if (bytebeam_client->action_funcs[action_iterator].name != NULL) {
            ESP_LOGI(TAG, "       {%s : %s}       \n", bytebeam_client->action_funcs[action_iterator].name, "*******");
        } else {
            ESP_LOGI(TAG, "       {%s : %s}       \n", "NULL", "NULL");
        }
    }
    ESP_LOGI(TAG, "]");
}

void bytebeam_reset_action_handler_array(bytebeam_client_t *bytebeam_client)
{
    int action_iterator = 0;

    for (action_iterator = 0; action_iterator < BYTEBEAM_NUMBER_OF_ACTIONS; action_iterator++) {
        bytebeam_client->action_funcs[action_iterator].func = NULL;
        bytebeam_client->action_funcs[action_iterator].name = NULL;
    }

    function_handler_index = 0;
}