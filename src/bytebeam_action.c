#include "cJSON.h"
#include "esp_log.h"
#include "sys/time.h"
#include "bytebeam_esp_hal.h"
#include "bytebeam_action.h"

static int function_handler_index = 0;
static char bytebeam_last_known_action_id[BYTEBEAM_ACTION_ID_STR_LEN] = { 0 };

static const char *TAG = "BYTEBEAM_ACTION";

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

bytebeam_err_t bytebeam_add_action_handler(bytebeam_client_t *bytebeam_client, int (*func_ptr)(bytebeam_client_t *, char *, char *), char *func_name)
{

    if (bytebeam_client == NULL || func_ptr == NULL || func_name == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    if (bytebeam_client == NULL || func_name == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    if (bytebeam_client == NULL || new_func_ptr == NULL || func_name == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    if (bytebeam_client == NULL || func_name == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

bytebeam_err_t bytebeam_print_action_handler_array(bytebeam_client_t *bytebeam_client)
{

    if (bytebeam_client == NULL) 
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_reset_action_handler_array(bytebeam_client_t *bytebeam_client)
{

    if (bytebeam_client == NULL) 
    {
        return BB_NULL_CHECK_FAILURE;
    }

    int action_iterator = 0;

    for (action_iterator = 0; action_iterator < BYTEBEAM_NUMBER_OF_ACTIONS; action_iterator++) {
        bytebeam_client->action_funcs[action_iterator].func = NULL;
        bytebeam_client->action_funcs[action_iterator].name = NULL;
    }

    function_handler_index = 0;

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_publish_action_completed(bytebeam_client_t *bytebeam_client, char *action_id)
{
    int ret_val = 0;

    if (bytebeam_client == NULL || action_id == NULL) 
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    if (bytebeam_client == NULL || action_id == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

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

    if (bytebeam_client == NULL || action_id == NULL) 
    {
        return BB_NULL_CHECK_FAILURE;
    }
    
    // Check if progress percentage is out of range
    if (progress_percentage < 0 || progress_percentage > 100) 
    {
        return BB_PROGRESS_OUT_OF_RANGE;
    }
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