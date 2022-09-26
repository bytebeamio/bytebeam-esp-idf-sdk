#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"

#include "bytebeam_actions.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
// #include "user_actions.h"

const char* ota_states[1] =
{
	"Dummy_data"
};

device_config test_device_config;
char* ota_action_id=" ";

static const char *TAG_BYTEBEAM_ACTIONS = "BYTEBEAM_SDK_ACTIONS";

action_functions_map action_funcs[NUMBER_OF_ACTIONS];

void initialize_action_handler_array(action_functions_map* action_handler_array)
{
	int loop_var=1;
	action_handler_array[0].func=handle_ota;
	action_handler_array[0].name="update_firmware";
	for(loop_var=1;loop_var<NUMBER_OF_ACTIONS;loop_var++)
	{
		action_handler_array[loop_var].func=NULL;
		action_handler_array[loop_var].name=NULL;
	}
}

int parse_ota_json(char*payload_string, char* url_string_return)
{    
	const cJSON *url = NULL;
	const cJSON *version = NULL;

	cJSON* pl_json = cJSON_Parse(payload_string);
	url=cJSON_GetObjectItem(pl_json, "url");
	
	if (cJSON_IsString(url) && (url->valuestring != NULL))
	{
		ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "Checking url \"%s\"\n", url->valuestring);
	}

	version = cJSON_GetObjectItem(pl_json, "version");

	if (cJSON_IsString(version) && (version->valuestring != NULL))
	{
		ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "Checking version \"%s\"\n", version->valuestring);
	}

	sprintf(url_string_return, "%s", url->valuestring);	
	ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "The constructed URL is: %s", url_string_return);
	
	return 0;
}


void publish_action_status(device_config device_cfg,char* action_id, int percentage, bytebeam_client_handle_t client, char* status, char* error_message)
{
	static uint64_t sq_num = 0;
	cJSON *demo_data_json_list = NULL;
	cJSON *demo_data_json = NULL;
	cJSON *percentage_json = NULL;
	cJSON *timestamp_json = NULL;
	cJSON *device_status_json = NULL;
	cJSON *action_id_json = NULL;
	cJSON *action_errors_json = NULL;
	cJSON *seq_json  = NULL;
	char *string_json = NULL;

	demo_data_json_list = cJSON_CreateArray();
	if (demo_data_json_list == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json Init failed.");
		goto END;
	} 
	demo_data_json = cJSON_CreateObject();
	if (demo_data_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add failed.");
		goto END;
	}	
	struct timeval te;
	sq_num++;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

	timestamp_json = cJSON_CreateNumber(milliseconds);
	if (timestamp_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add time stamp failed.");
		goto END;
	}
	cJSON_AddItemToObject(demo_data_json, "timestamp", timestamp_json);

	seq_json = cJSON_CreateNumber(sq_num);
	if (timestamp_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add time stamp failed.");
		goto END;
	}
	cJSON_AddItemToObject(demo_data_json, "sequence", seq_json);

	device_status_json = cJSON_CreateString(status);
	if(device_status_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add device status failed.");
		goto END;
	}
	
	cJSON_AddItemToObject(demo_data_json, "state", device_status_json);

	ota_states[0]=error_message;
	//action_errors_json = cJSON_CreateString(error_message);
	action_errors_json = cJSON_CreateStringArray(ota_states,1);
	if(action_errors_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add device status failed.");
		goto END;
	}
	cJSON_AddItemToObject(demo_data_json, "errors", action_errors_json);

	action_id_json = cJSON_CreateString(action_id);
	if(action_id_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add device status failed.");
		goto END;
	}
	cJSON_AddItemToObject(demo_data_json, "id", action_id_json);

	percentage_json = cJSON_CreateNumber(percentage);
	if (percentage_json == NULL)
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS,"Json add time stamp failed.");
		goto END;
	}
	cJSON_AddItemToObject(demo_data_json, "progress", percentage_json);

	cJSON_AddItemToArray(demo_data_json_list, demo_data_json);

	string_json =  cJSON_Print(demo_data_json_list);
	ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "\nTrying to print:\n%s\n", string_json);
	char topic[300] = {0,};
	sprintf(topic,"/tenants/%s/devices/%s/action/status", device_cfg.project_id, device_cfg.device_id );
	ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "\n%s\n", topic);
	int msg_id =  bytebeam_hal_mqtt_publish(client, topic, string_json, strlen(string_json),  1);
	if(msg_id!=-1)
	{
		ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "sent publish successful, msg_id=%d, message:%s", msg_id, string_json);
	}
	else
	{
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS, "Publish Failed");
	}
	END:
	cJSON_Delete(demo_data_json_list);
	free(string_json);
}


void perform_ota(device_config device_cfg, char* action_id, char* ota_url, bytebeam_client_handle_t client)
{
	test_device_config=device_cfg;
	ESP_LOGI(TAG_BYTEBEAM_ACTIONS, "Starting OTA.....");
	esp_err_t ret = bytebeam_hal_ota(&device_cfg, ota_url,client); 
	if (ret == ESP_OK) {
		nvs_handle_t nvs_handle;
		int32_t update_flag=1;
		int32_t action_id_val=(int32_t)(atoi(ota_action_id));
		nvs_flash_init();
		nvs_open("test_storage",NVS_READWRITE,&nvs_handle);
		nvs_set_i32(nvs_handle,"update_flag",update_flag);
		nvs_commit(nvs_handle);
		nvs_set_i32(nvs_handle,"action_id_val",action_id_val);
		nvs_commit(nvs_handle);
		nvs_close(nvs_handle);
		bytebeam_hal_restart(NULL);
	} 
	else 
	{
		publish_action_status(device_cfg,action_id,0, client, "Failed", "Failed");
		ESP_LOGE(TAG_BYTEBEAM_ACTIONS, "Firmware Upgrade Failed");
	}
}

int handle_ota(bytebeam_client *bb_obj, char *payload_string, char *action_id)
{
    char constrcuted_url[200] = {0,};
    parse_ota_json(payload_string, constrcuted_url);
	ota_action_id=action_id;
    perform_ota(bb_obj->device_cfg, action_id, constrcuted_url, bb_obj->client);
    return 0;
}