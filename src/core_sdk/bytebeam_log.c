#include "cJSON.h"
#include "bytebeam_hal.h"
#include "bytebeam_stream.h"
#include "bytebeam_log.h"

// bytebeam log module variables
static bool is_cloud_logging_enable = false;
static char bytebeam_log_stream[BYTEBEAM_LOG_STREAM_STR_LEN] = "";
static bytebeam_log_level_t bytebeam_log_level = BYTEBEAM_LOG_LEVEL_INFO;
static bytebeam_client_t *bytebeam_log_client = NULL;

static const char *TAG = "BYTEBEAM_LOG";

void bytebeam_log_client_set(bytebeam_client_t *bytebeam_client)
{
    bytebeam_log_client = bytebeam_client;
}

void bytebeam_enable_cloud_logging()
{
    BB_LOGI(TAG, "Cloud Logging Enabled.");
    is_cloud_logging_enable = true;
}

bool bytebeam_is_cloud_logging_enabled()
{
    return is_cloud_logging_enable;
}

void bytebeam_disable_cloud_logging()
{
    BB_LOGI(TAG, "Cloud Logging Disabled.");
    is_cloud_logging_enable = false;
}

void bytebeam_log_level_set(bytebeam_log_level_t level)
{
    BB_LOGI(TAG, "Setting Bytebeam Log Level : %d", level);
    bytebeam_log_level = level;
}

bytebeam_log_level_t bytebeam_log_level_get(void)
{
    return bytebeam_log_level;
}

bytebeam_err_t bytebeam_log_stream_set(char* stream_name)
{
    if (stream_name == NULL)
    {
        return BB_NULL_CHECK_FAILURE;
    }

    int max_len = BYTEBEAM_LOG_STREAM_STR_LEN;
    int temp_var = snprintf(bytebeam_log_stream, max_len, "%s", stream_name);

    if (temp_var >= max_len)
    {
        BB_LOGE(TAG, "log stream size exceeded buffer size");
        return BB_FAILURE;
    }

    BB_LOGI(TAG, "Setting Bytebeam Cloud Logging Stream : %s", stream_name);

    return BB_SUCCESS;
}

char* bytebeam_log_stream_get()
{
    return bytebeam_log_stream;
}

bytebeam_err_t bytebeam_log_publish(const char *level, const char *tag, const char *fmt, ...)
{
    static uint64_t sequence = 0;
    unsigned long long milliseconds = 0;

    cJSON *device_log_json_list = NULL;
    cJSON *device_log_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *log_level_json = NULL;
    cJSON *log_tag_json = NULL;
    cJSON *log_message_json = NULL;

    char *log_string_json = NULL;

    if(bytebeam_log_client == NULL)
    {
        BB_LOGE(TAG, "Bytebeam log client handle is not set");
        return BB_FAILURE;
    }

    // if cloud logging is disabled, we don't need to push just return :)
    if(!is_cloud_logging_enable) 
    {
        BB_LOGE(TAG, "Bytebeam cloud logging is not enabled");
        return BB_FAILURE;
    }

    va_list args;

    // get the buffer size (+1 for NULL Character)
    va_start(args, fmt);
    int buffer_size = vsnprintf(NULL, 0, fmt, args) + 1; 
    va_end(args);

    // Check the buffer size
    if(buffer_size <= 0) 
    {
        BB_LOGE(TAG, "Failed to Get the buffer size for Bytebeam Log.");
        return BB_FAILURE;
    }

    // allocate the memory for the buffer to store the message
    char *message_buffer = (char*) malloc(buffer_size);
    if (message_buffer == NULL) 
    {
        BB_LOGE(TAG, "Failed to ALlocate the memory for Bytebeam Log.");
        return BB_FAILURE;
    }

    // get the message in the buffer
    va_start(args, fmt);
    int temp_var = vsnprintf(message_buffer, buffer_size, fmt, args);
    va_end(args);

    // Check for argument loss
    if (temp_var >= buffer_size) 
    {
        BB_LOGE(TAG, "Failed to Get the message for Bytebeam Log.");

        free(message_buffer);
        return BB_FAILURE;
    }

    device_log_json_list = cJSON_CreateArray();

    if (device_log_json_list == NULL) {
        BB_LOGE(TAG, "Log Json Init failed.");

        free(message_buffer);
        return BB_FAILURE;
    }

    device_log_json = cJSON_CreateObject();

    if (device_log_json == NULL) {
        BB_LOGE(TAG, "Log Json add failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }
    
    milliseconds = bytebeam_hal_get_epoch_millis();

    if(milliseconds == 0)
    {
        BB_LOGE(TAG, "failed to get epoch millis.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if (timestamp_json == NULL) {
        BB_LOGE(TAG, "Log Json add time stamp failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if (sequence_json == NULL) {
        BB_LOGE(TAG, "Log Json add sequence id failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "sequence", sequence_json);

    log_level_json = cJSON_CreateString(level);

    if (log_level_json == NULL) {
        BB_LOGE(TAG, "Log Json add level failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "level", log_level_json);

    log_tag_json = cJSON_CreateString(tag);

    if (log_tag_json == NULL) {
        BB_LOGE(TAG, "Log Json add tag failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "tag", log_tag_json);

    log_message_json = cJSON_CreateString(message_buffer);

    if (log_message_json == NULL) {
        BB_LOGE(TAG, "Log Json add message failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "message", log_message_json);

    cJSON_AddItemToArray(device_log_json_list, device_log_json);

    log_string_json = cJSON_Print(device_log_json_list);

    if(log_string_json == NULL)
    {
        BB_LOGE(TAG, "Json string print failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    } 

    BB_LOGD(TAG, "\n Log to Send :\n%s\n", log_string_json);

    int ret_val = bytebeam_publish_to_stream(bytebeam_log_client, bytebeam_log_stream, log_string_json);

    cJSON_Delete(device_log_json_list);
    cJSON_free(log_string_json);
    free(message_buffer);

    return ret_val;
}