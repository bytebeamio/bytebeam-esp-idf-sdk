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
#include "esp_idf_version.h"

char *ota_action_id = "";
char ota_error_str[BYTEBEAM_OTA_ERROR_STR_LEN] = "";

static const char *TAG = "BYTEBEAM_SDK";

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