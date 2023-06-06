#include "cJSON.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"
#include "bytebeam_log.h"
#include "bytebeam_client.h"

static cJSON *bytebeam_cert_json = NULL;
static char *bytebeam_device_config_data = NULL;

static const char *TAG = "BYTEBEAM_CLIENT";

static int read_device_config_file()
{
    char config_fname[100] = "";
    esp_err_t ret_code = ESP_OK;

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_SPIFFS
    ESP_LOGI(TAG, "SPIFFS file system detected !");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    // initalize the SPIFFS file system
    ret_code = esp_vfs_spiffs_register(&conf);

    if (ret_code != ESP_OK)
    {
        switch(ret_code)
        {
            case ESP_FAIL:
                ESP_LOGE(TAG, "Failed to mount or format SPIFFS");
                break;

            case ESP_ERR_NOT_FOUND:
                ESP_LOGE(TAG, "Unable to find SPIFFS partition");
                break;

            default:
                ESP_LOGE(TAG, "Failed to register SPIFFS partition (%s)", esp_err_to_name(ret_code));
        }

        return -1;
    }

    // generate the device config file name
    strcat(config_fname, "/spiffs/");
    strcat(config_fname, CONFIG_BYTEBEAM_PROVISIONING_FILENAME);
#endif

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_LITTLEFS
    ESP_LOGI(TAG, "LITTLEFS file system detected !");

    // Just print the log and return :)
    ESP_LOGI(TAG, "LITTLEFS file system is not supported by the sdk yet :)");
    return -1;
#endif

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_FATFS
    ESP_LOGI(TAG, "FATFS file system detected !");

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

        return -1;
    }

    // generate the device config file name
    strcat(config_fname, "/spiflash/");
    strcat(config_fname, CONFIG_BYTEBEAM_PROVISIONING_FILENAME);
#endif

    const char* path = config_fname;
    ESP_LOGI(TAG, "Reading file : %s", path);

    FILE *file = fopen(path, "r");

    if (file == NULL) 
    {
        ESP_LOGE(TAG, "Failed to open device config file for reading");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(file_length <= 0)
    {
        ESP_LOGE(TAG, "Failed to get device config file size");
        return -1;
    }

    // dynamically allocate a char array to store the file contents
    bytebeam_device_config_data = malloc(sizeof(char) * (file_length + 1));

    // if memory allocation fails just log the failure to serial and return :)
    if(bytebeam_device_config_data == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate the memory for device config file");
        return -1;
    }

    int temp_c;
    int loop_var = 0;

    while ((temp_c = fgetc(file)) != EOF)
    {
        bytebeam_device_config_data[loop_var] = temp_c;
        loop_var++;
    }

    bytebeam_device_config_data[loop_var] = '\0';

    fclose(file);

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_SPIFFS
    // de-initalize the SPIFFS file system
    ret_code = esp_vfs_spiffs_unregister(conf.partition_label);

    if (ret_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unregister SPIFFS (%s)", esp_err_to_name(ret_code));
        return -1;
    }
#endif

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_LITTLEFS
    // de-initalize the LITTLEFS file system
    // nothing to do here yet

#endif

#if CONFIG_BYTEBEAM_PROVISION_DEVICE_FROM_FATFS
    // de-initalize the FATFS file system
    ret_code = esp_vfs_fat_spiflash_unmount_ro("/spiflash", "storage");

    if (ret_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unregister FATFS (%s)", esp_err_to_name(ret_code));
        return -1;
    }
#endif

    return 0;
}

static int parse_device_config_file(bytebeam_device_config_t *device_cfg)
{
    // before going ahead make sure you are parsing something
    if (bytebeam_device_config_data == NULL) {
        ESP_LOGE(TAG, "device config file is empty");

        free(bytebeam_device_config_data);
        return -1;
    }

    // refer to json file to the device config data
    const char *json_file = (const char *)bytebeam_device_config_data;

    /*  Do not delete the bytebeam cert json object from memory because we are giving the reference of the certificates to the mqtt
     *  library (see below), so it needs to be there in the memory. Ofcourse we will delete the json object to release the
     *  memory and that is handled properly in bytebeam sdk cleanup.
     */

    bytebeam_cert_json = cJSON_Parse(json_file);

    if (bytebeam_cert_json == NULL) {
        ESP_LOGE(TAG, "ERROR in parsing the JSON\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *prj_id_obj = cJSON_GetObjectItem(bytebeam_cert_json, "project_id");

    if (!(cJSON_IsString(prj_id_obj) && (prj_id_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR in getting the project id\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    int max_len = BYTEBEAM_PROJECT_ID_STR_LEN;
    int temp_var = snprintf(device_cfg->project_id, max_len, "%s", prj_id_obj->valuestring);

    if(temp_var >= max_len)
    {   
        ESP_LOGE(TAG, "Project Id length exceeded buffer size");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *broker_name_obj = cJSON_GetObjectItem(bytebeam_cert_json, "broker");

    if (!(cJSON_IsString(broker_name_obj) && (broker_name_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR parsing broker name");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *port_num_obj = cJSON_GetObjectItem(bytebeam_cert_json, "port");

    if (!(cJSON_IsNumber(port_num_obj))) {
        ESP_LOGE(TAG, "ERROR parsing port number.");

        free(bytebeam_device_config_data);
        return -1;
    }

    int port_int = port_num_obj->valuedouble;

    max_len = BYTEBEAM_BROKER_URL_STR_LEN;
    temp_var = snprintf(device_cfg->broker_uri, max_len, "mqtts://%s:%d", broker_name_obj->valuestring, port_int);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "Broker URL length exceeded buffer size");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *device_id_obj = cJSON_GetObjectItem(bytebeam_cert_json, "device_id");

    if (!(cJSON_IsString(device_id_obj) && (device_id_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR parsing device id\n");

        free(bytebeam_device_config_data);
        return -1;
    }
    
    max_len = BYTEBEAM_DEVICE_ID_STR_LEN;
    temp_var = snprintf(device_cfg->device_id, max_len, "%s", device_id_obj->valuestring);
    
    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "Device Id length exceeded buffer size");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *auth_obj = cJSON_GetObjectItem(bytebeam_cert_json, "authentication");

    if (bytebeam_cert_json == NULL || !(cJSON_IsObject(auth_obj))) {
        ESP_LOGE(TAG, "ERROR in parsing the auth JSON\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    cJSON *ca_cert_obj = cJSON_GetObjectItem(auth_obj, "ca_certificate");

    if (!(cJSON_IsString(ca_cert_obj) && (ca_cert_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR parsing ca certificate\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    device_cfg->ca_cert_pem = (char *)ca_cert_obj->valuestring;

    cJSON *device_cert_obj = cJSON_GetObjectItem(auth_obj, "device_certificate");

    if (!(cJSON_IsString(device_cert_obj) && (device_cert_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR parsing device certifate\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    device_cfg->client_cert_pem = (char *)device_cert_obj->valuestring;

    cJSON *device_private_key_obj = cJSON_GetObjectItem(auth_obj, "device_private_key");

    if (!(cJSON_IsString(device_private_key_obj) && (device_private_key_obj->valuestring != NULL))) {
        ESP_LOGE(TAG, "ERROR parsing device private key\n");

        free(bytebeam_device_config_data);
        return -1;
    }

    device_cfg->client_key_pem = (char *)device_private_key_obj->valuestring;

    free(bytebeam_device_config_data);
    bytebeam_device_config_data = NULL;

    return 0;
}

static void set_mqtt_conf(bytebeam_device_config_t *device_cfg, bytebeam_client_config_t *mqtt_cfg)
{
    // set the broker uri
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mqtt_cfg->broker.address.uri = device_cfg->broker_uri;
    ESP_LOGI(TAG, "The uri  is: %s\n", mqtt_cfg->broker.address.uri);
#else
    mqtt_cfg->uri = device_cfg->broker_uri;
    ESP_LOGI(TAG, "The uri  is: %s\n", mqtt_cfg->uri);
#endif  

    // set the server certificate
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mqtt_cfg->broker.verification.certificate = (const char *)device_cfg->ca_cert_pem;
#else
    mqtt_cfg->cert_pem = (const char *)device_cfg->ca_cert_pem;
#endif

    // set the client certificate
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mqtt_cfg->credentials.authentication.certificate = (const char *)device_cfg->client_cert_pem;
#else
    mqtt_cfg->client_cert_pem = (const char *)device_cfg->client_cert_pem;
#endif

    // set the client key
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    mqtt_cfg->credentials.authentication.key = (const char *)device_cfg->client_key_pem;
#else
    mqtt_cfg->client_key_pem = (const char *)device_cfg->client_key_pem;
#endif
}

static void bytebeam_sdk_cleanup(bytebeam_client_t *bytebeam_client)
{
    /* We will use this function in bytebeam client init and bytebeam client destroy phase, So to make sure if
     * bytebeam client is not running then we don't have any memory leaks (mostly solving pointer issues) :)
     */

    ESP_LOGD(TAG, "Cleaning Up Bytebeam SDK");

    // clearing bytebeam device configuration
    bytebeam_client->device_cfg.ca_cert_pem = NULL;
    bytebeam_client->device_cfg.client_cert_pem = NULL;
    bytebeam_client->device_cfg.client_key_pem = NULL;
    memset(bytebeam_client->device_cfg.broker_uri, 0x00, sizeof(bytebeam_client->device_cfg.broker_uri));
    memset(bytebeam_client->device_cfg.device_id , 0x00, sizeof(bytebeam_client->device_cfg.device_id));
    memset(bytebeam_client->device_cfg.project_id, 0x00, sizeof(bytebeam_client->device_cfg.project_id));

    // clearing bytebeam mqtt client
    bytebeam_client->client = NULL;

    // clearing bytebeam mqtt configuration
    memset(&(bytebeam_client->mqtt_cfg), 0x00, sizeof(bytebeam_client->mqtt_cfg));

    // clearing bytebeam action functions array
    bytebeam_reset_action_handler_array(bytebeam_client);

    // clearing bytebeam connection status
    bytebeam_client->connection_status = 0;

    // clearing OTA action id
    // ota_action_id = NULL;

    // clearing bytebeam log client
    bytebeam_log_client_set(NULL);

    // clearing bytebeam log level
    bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL_NONE);

    // clearing certifciate json object
    if(bytebeam_cert_json != NULL) {
        cJSON_Delete(bytebeam_cert_json);
        bytebeam_cert_json = NULL;
        ESP_LOGD(TAG, "Certificate JSON object deleted");
    }

    ESP_LOGD(TAG, "Bytebeam SDK Cleanup done !!");

}

bytebeam_err_t bytebeam_init(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;
    
    // check-in the device config data from file system if in case not provided 
    if (bytebeam_client->use_device_config_data == false) {
        // read the device config json stored in file system
        ret_val = read_device_config_file();

        if(ret_val != 0) {
            ESP_LOGE(TAG, "Error in reading device config JSON");

            /* This call will clear all the bytebeam sdk variables so to avoid any memory leaks further */
            bytebeam_sdk_cleanup(bytebeam_client);
            return BB_FAILURE;
        }

        // parse the device config json readed from the file system
        ret_val = parse_device_config_file(&(bytebeam_client->device_cfg));

        if (ret_val != 0) {
            ESP_LOGE(TAG, "Error in parsing device config JSON");

            /* This call will clear all the bytebeam sdk variables so to avoid any memory leaks further */
            bytebeam_sdk_cleanup(bytebeam_client);
            return BB_FAILURE;
        }
    } else {
        ESP_LOGI(TAG, "Using provided device config data !");
    }

    // set the mqtt configurations
    set_mqtt_conf(&(bytebeam_client->device_cfg), &(bytebeam_client->mqtt_cfg));

    // initialize the bytebeam hal layer
    ret_val = bytebeam_hal_init(bytebeam_client);

    if (ret_val != 0) {
        ESP_LOGE(TAG, "Error in initializing bytebeam hal");

        /* This call will clear all the bytebeam sdk variables so to avoid any memory leaks further */
        bytebeam_sdk_cleanup(bytebeam_client);
        return BB_FAILURE;
    }

    bytebeam_log_client_set(bytebeam_client);
    bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL);

    ESP_LOGI(TAG, "Bytebeam Client Initialized !!");

    return BB_SUCCESS;
}

bytebeam_err_t bytebeam_start(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;

    ret_val = bytebeam_hal_start_mqtt(bytebeam_client);

    if (ret_val != 0) {
        ESP_LOGE(TAG, "Bytebeam Client start failed");
        return BB_FAILURE;
    } else {
        ESP_LOGI(TAG, "Bytebeam Client started !!");
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_stop(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;

    ret_val = bytebeam_hal_stop_mqtt(bytebeam_client);

    if (ret_val != 0) {
        ESP_LOGE(TAG, "Bytebam Client stop failed");
        return BB_FAILURE;
    } else {
        ESP_LOGI(TAG, "Bytebeam Client stopped !!");
        return BB_SUCCESS;
    }
}

bytebeam_err_t bytebeam_destroy(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;

    ret_val = bytebeam_hal_destroy(bytebeam_client);

    if (ret_val != 0) {
        ESP_LOGE(TAG, "Bytebeam Client destroy failed");
        return BB_FAILURE;
    }

    /* This call will clearing all the bytebeam sdk variables so to avoid any memory leaks further */
    bytebeam_sdk_cleanup(bytebeam_client);

    ESP_LOGI(TAG, "Bytebeam Client destroyed !!");

    return BB_SUCCESS;
}