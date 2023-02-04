/**
 * Brief
 * This app creates storage partition in SPIFFS and stores config data file
 */

#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_spiffs.h"

// Device Provisioning Success Code
#define BYTEBEAM_PROVISIONING_SUCCESS 0

// Device Provisioning Failure Code
#define BYTEBEAM_PROVISIONING_FAILURE -1

/* This macro is used to log the device config data to serial monitor i.e set to debug any issue */
#define LOG_DEVICE_CONFIG_DATA_TO_SERIAL false

static const char *TAG = "BYTEBEAM_PROVISIONING_EXAMPLE";

static char *utils_read_file(char *filename)
{
    FILE *file;

    file = fopen(filename, "r");

    if (file == NULL)
    {
        ESP_LOGE(TAG, "Fialed to open device config file for reading");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(file_length <= 0)
    {
        ESP_LOGE(TAG, "Fialed to get device config file size");
        return NULL;
    }

    // dynamically allocate a char array to store the file contents
    char *buff = (char *) malloc(sizeof(char) * (file_length + 1));

    if(buff == NULL)
    {
        ESP_LOGE(TAG, "Fialed to allocate the memory for device config file");
        return NULL;
    }

    int temp_c;
    int loop_var = 0;

    while ((temp_c = fgetc(file)) != EOF)
    {
        buff[loop_var] = temp_c;
        loop_var++;
    }

    buff[loop_var] = '\0';

    fclose(file);

    return buff;
}

static int read_device_config_file(void)
{
    esp_err_t ret_code = ESP_OK;
    char *config_fname = "/spiffs/device_config.json";

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret_code = esp_vfs_spiffs_register(&conf);

    if(ret_code != ESP_OK)
    {
        if(ret_code == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if(ret_code == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to register SPIFFS partition");
        }

        return BYTEBEAM_PROVISIONING_FAILURE;
    }

    char *device_config_data = utils_read_file(config_fname);

    if(device_config_data == NULL)
    {
        ESP_LOGE(TAG, "Error in fetching Config data from FLASH");

        ret_code = esp_vfs_spiffs_unregister(conf.partition_label);

        if (ret_code != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to unregister SPIFFS partition");
        }

        free(device_config_data);

        return BYTEBEAM_PROVISIONING_FAILURE;
    }

    ret_code = esp_vfs_spiffs_unregister(conf.partition_label);

    if(ret_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unregister SPIFFS partition");
        return BYTEBEAM_PROVISIONING_FAILURE;
    }

#if LOG_DEVICE_CONFIG_DATA_TO_SERIAL
    ESP_LOGI(TAG, "device_config_data :");
    ESP_LOGI(TAG, "%s", device_config_data);
#endif

    free(device_config_data);

    return BYTEBEAM_PROVISIONING_SUCCESS;
}

void app_main()
{
    if(read_device_config_file() == BYTEBEAM_PROVISIONING_SUCCESS)
    {
        ESP_LOGI(TAG, "Device Provisioning Success.");
    }
    else
    {
        ESP_LOGI(TAG, "Device Provisioning Failure.");
    }
}
