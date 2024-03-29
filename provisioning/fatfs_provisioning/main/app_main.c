/**
 * Brief
 * This app creates storage partition in SPIFFS and stores config data file
 */

#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"

// Device Provisioning Success Code
#define BYTEBEAM_PROVISIONING_SUCCESS 0

// Device Provisioning Failure Code
#define BYTEBEAM_PROVISIONING_FAILURE -1

/* This macro is used to log the device config data to serial monitor i.e set to debug any issue */
#define LOG_DEVICE_CONFIG_DATA_TO_SERIAL false

static const char *TAG = "BYTEBEAM_PROVISIONING_EXAMPLE";

static char *utils_read_file(char *filename)
{
    const char* path = filename;
    ESP_LOGI(TAG, "Reading file : %s", path);

    FILE *file = fopen(path, "r");

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
    char *config_fname = "/spiflash/deviceconfig.json";

    const esp_vfs_fat_mount_config_t conf = {
            .max_files = 4,
            .format_if_mount_failed = false,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    // initalize the FATFS file system
    ret_code = esp_vfs_fat_spiflash_mount_ro("/spiflash", "storage", &conf);

    if (ret_code != ESP_OK)
    {
        switch(ret_code)
        {
            case ESP_FAIL:
                ESP_LOGE(TAG, "Failed to mount FATFS");
                break;

            case ESP_ERR_NOT_FOUND:
                ESP_LOGE(TAG, "Unable to find FATFS partition");
                break;

            default:
                ESP_LOGE(TAG, "Failed to register FATFS (%s)", esp_err_to_name(ret_code));
        }

        return BYTEBEAM_PROVISIONING_FAILURE;
    }

    // read the device config data
    char *device_config_data = utils_read_file(config_fname);

    // de-initalize the FATFS file system
    ret_code = esp_vfs_fat_spiflash_unmount_ro("/spiflash", "storage");

    if (ret_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unregister FATFS (%s)", esp_err_to_name(ret_code));

        free(device_config_data);
        return BYTEBEAM_PROVISIONING_FAILURE;
    }

    if(device_config_data == NULL)
    {   
        ESP_LOGE(TAG, "Error in fetching Config data from FLASH");

        free(device_config_data);
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
