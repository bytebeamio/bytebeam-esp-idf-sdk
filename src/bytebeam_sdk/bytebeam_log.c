#include "cJSON.h"
#include "esp_log.h"
#include "sys/time.h"
#include "bytebeam_sdk.h"
#include "bytebeam_log.h"

// bytebeam log module variables
static bool is_cloud_logging_enable = true;
static bytebeam_client_t *bytebeam_log_client = NULL;
static char bytebeam_log_stream[BYTEBEAM_LOG_STREAM_STR_LEN] = "logs";
static bytebeam_log_level_t bytebeam_log_level = BYTEBEAM_LOG_LEVEL_NONE;

static const char *TAG = "BYTEBEAM_LOG";

void bytebeam_log_client_set(bytebeam_client_t *bytebeam_client)
{
    bytebeam_log_client = bytebeam_client;
}

void bytebeam_enable_cloud_logging()
{
    is_cloud_logging_enable = true;
}

bool bytebeam_is_cloud_logging_enabled()
{
    return is_cloud_logging_enable;
}

void bytebeam_disable_cloud_logging()
{
    is_cloud_logging_enable = false;
}

void bytebeam_log_level_set(bytebeam_log_level_t level)
{
    bytebeam_log_level = level;
}

bytebeam_log_level_t bytebeam_log_level_get(void)
{
    return bytebeam_log_level;
}

void bytebeam_log_stream_set(char* stream_name)
{
    int max_len = BYTEBEAM_LOG_STREAM_STR_LEN;
    int temp_var = snprintf(bytebeam_log_stream, max_len, "%s", stream_name);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "log stream size exceeded buffer size");
    }
}

char* bytebeam_log_stream_get()
{
    return bytebeam_log_stream;
}

bytebeam_err_t bytebeam_log_publish(const char *level, const char *tag, const char *fmt, ...)
{
    static uint64_t sequence = 0;

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
        ESP_LOGE(TAG, "Bytebeam log client handle is not set");
        return BB_FAILURE;
    }

    // if cloud logging is disabled, we don't need to push just return :)
    if(!is_cloud_logging_enable) {
      return BB_SUCCESS;
    }

    va_list args;

    // get the buffer size (+1 for NULL Character)
    va_start(args, fmt);
    int buffer_size = vsnprintf(NULL, 0, fmt, args) + 1; 
    va_end(args);

    // Check the buffer size
    if(buffer_size <= 0) 
    {
        ESP_LOGE(TAG, "Failed to Get the buffer size for Bytebeam Log.");
        return BB_FAILURE;
    }

    // allocate the memory for the buffer to store the message
    char *message_buffer = (char*) malloc(buffer_size);
    if (message_buffer == NULL) 
    {
        ESP_LOGE(TAG, "Failed to ALlocate the memory for Bytebeam Log.");
        return BB_FAILURE;
    }

    // get the message in the buffer
    va_start(args, fmt);
    int temp_var = vsnprintf(message_buffer, buffer_size, fmt, args);
    va_end(args);

    // Check for argument loss
    if (temp_var >= buffer_size) 
    {
        ESP_LOGE(TAG, "Failed to Get the message for Bytebeam Log.");

        free(message_buffer);
        return BB_FAILURE;
    }

    device_log_json_list = cJSON_CreateArray();

    if (device_log_json_list == NULL) {
        ESP_LOGE(TAG, "Log Json Init failed.");

        free(message_buffer);
        return BB_FAILURE;
    }

    device_log_json = cJSON_CreateObject();

    if (device_log_json == NULL) {
        ESP_LOGE(TAG, "Log Json add failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }
    
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if (timestamp_json == NULL) {
        ESP_LOGE(TAG, "Log Json add time stamp failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if (sequence_json == NULL) {
        ESP_LOGE(TAG, "Log Json add sequence id failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "sequence", sequence_json);

    log_level_json = cJSON_CreateString(level);

    if (log_level_json == NULL) {
        ESP_LOGE(TAG, "Log Json add level failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "level", log_level_json);

    log_tag_json = cJSON_CreateString(tag);

    if (log_tag_json == NULL) {
        ESP_LOGE(TAG, "Log Json add tag failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "tag", log_tag_json);

    log_message_json = cJSON_CreateString(message_buffer);

    if (log_message_json == NULL) {
        ESP_LOGE(TAG, "Log Json add message failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    }

    cJSON_AddItemToObject(device_log_json, "message", log_message_json);

    cJSON_AddItemToArray(device_log_json_list, device_log_json);

    log_string_json = cJSON_Print(device_log_json_list);

    if(log_string_json == NULL)
    {
        ESP_LOGE(TAG, "Json string print failed.");

        cJSON_Delete(device_log_json_list);
        free(message_buffer);
        return BB_FAILURE;
    } 

    ESP_LOGD(TAG, "\n Log to Send :\n%s\n", log_string_json);

    int ret_val = bytebeam_publish_to_stream(bytebeam_log_client, bytebeam_log_stream, log_string_json);

    cJSON_Delete(device_log_json_list);
    cJSON_free(log_string_json);
    free(message_buffer);

    return ret_val;
}