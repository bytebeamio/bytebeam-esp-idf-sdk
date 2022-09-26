#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_log.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"
#include "bytebeam_actions.h"

extern const uint8_t device_cert_json_start[] asm("_binary_device_1222_json_start");
extern const uint8_t device_cert_json_end[] asm("_binary_device_1222_json_end");

static const char *TAG = "BYTEBEAM_SDK";

int subscribe_to_actions(device_config device_cfg, bytebeam_client_handle_t client)
{
	int msg_id;
	char topic[200]={0,};
	sprintf(topic,"/tenants/%s/devices/%s/actions", device_cfg.project_id, device_cfg.device_id );
	msg_id = bytebeam_hal_mqtt_subscribe(client, topic, 1);
	ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d to topic=%s", msg_id, topic);
	return msg_id;
}


int parse_device_config_file(device_config *device_cfg, esp_mqtt_client_config_t *mqtt_cfg)
{
	const char *json_file = (const char *)device_cert_json_start;
	//ESP_LOGI(TAG, "[APP] The json file is: %s\n", json_file);

	cJSON *cert_json = cJSON_Parse(json_file); 

	if(cert_json == NULL)
	{
		ESP_LOGE(TAG, "ERROR in parsing the JSON\n");
		return -1;
	}

	cJSON *prj_id_obj = cJSON_GetObjectItem(cert_json, "project_id");
	if(!(cJSON_IsString(prj_id_obj) && (prj_id_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR in getting the prject id\n");
		return -1;
	}
	strcpy(device_cfg->project_id, prj_id_obj->valuestring);

	cJSON *broker_name_obj = cJSON_GetObjectItem(cert_json, "broker");
	if(!(cJSON_IsString(broker_name_obj) && (broker_name_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing broker name");
		return -1;
	}

	cJSON *port_num_obj = cJSON_GetObjectItem(cert_json, "port");
	if(!(cJSON_IsNumber(port_num_obj)))
	{
		ESP_LOGE(TAG, "ERROR parsing port number.");
		return -1;
	}

	int port_int = port_num_obj->valuedouble;
	sprintf(device_cfg->broker_uri,"mqtts://%s:%d", broker_name_obj->valuestring, port_int);
	mqtt_cfg->uri = device_cfg->broker_uri;

	ESP_LOGI(TAG, "The uri  is: %s\n", mqtt_cfg->uri);

	cJSON *device_id_obj = cJSON_GetObjectItem(cert_json, "device_id");
	if(!(cJSON_IsString(device_id_obj) && (device_id_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing device id\n");
		return -1;
	}

	strcpy(device_cfg->device_id, device_id_obj->valuestring);
	cJSON *auth_obj = cJSON_GetObjectItem(cert_json, "authentication");
	if(cert_json == NULL || !(cJSON_IsObject(auth_obj)))
	{
		ESP_LOGE(TAG, "ERROR in parsing the auth JSON\n");
		return -1;
	}

	cJSON *ca_cert_obj = cJSON_GetObjectItem(auth_obj, "ca_certificate");
	if(!(cJSON_IsString(ca_cert_obj) && (ca_cert_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing ca certificate\n");
		return -1;
	}

	device_cfg->ca_cert_pem = (char*)ca_cert_obj->valuestring; 
	mqtt_cfg->cert_pem =device_cfg->ca_cert_pem;

	cJSON *device_cert_obj = cJSON_GetObjectItem(auth_obj, "device_certificate");
	if(!(cJSON_IsString(device_cert_obj) && (device_cert_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing device certifate\n");
		return -1;
	}

	device_cfg->client_cert_pem = (char *)device_cert_obj-> valuestring;
	mqtt_cfg->client_cert_pem = device_cfg->client_cert_pem;

	cJSON *device_private_key_obj = cJSON_GetObjectItem(auth_obj, "device_private_key");
	if(!(cJSON_IsString(device_private_key_obj) && (device_private_key_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR device private key\n");
		return -1;
	}

	device_cfg->client_key_pem = (char *)device_private_key_obj->valuestring;
	mqtt_cfg->client_key_pem = (char*) device_cfg->client_key_pem;
	return 0;	
}


int handle_actions(char* action_received, bytebeam_client_handle_t client, bytebeam_client *bb_obj)
{
	 cJSON *root = NULL;
	 cJSON *name = NULL;
	 cJSON *payload = NULL;
	 cJSON *action_id_obj = NULL;
	char action_id[20] = {0,};
	int action_iterator = 0;

	root = cJSON_Parse(action_received);
	name = cJSON_GetObjectItem(root,"name");
	
	if (!(cJSON_IsString(name) && (name->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "Error parsing action name\n");
	}
	
	ESP_LOGI(TAG, "Checking name \"%s\"\n", name->valuestring);
	
	action_id_obj = cJSON_GetObjectItem(root, "id");
	strcpy(action_id, action_id_obj->valuestring);
	
	if (cJSON_IsString(action_id_obj) && (action_id_obj->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking version \"%s\"\n", action_id_obj->valuestring);
	}
	
	payload = cJSON_GetObjectItem(root, "payload");
		
	if (cJSON_IsString(payload) && (payload->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking payload \"%s\"\n", payload->valuestring);
		while(action_funcs[action_iterator].name)
		{
			if(!strcmp(action_funcs[action_iterator].name, name->valuestring))
			{
				action_funcs[action_iterator].func(bb_obj, payload->valuestring, action_id);
				break;
			}
			action_iterator++;
		}
		if(action_funcs[action_iterator].name == NULL)
		{
			ESP_LOGI(TAG, "Invalid action:%s\n", name->valuestring);
		}
	}

	free(payload);
	free(name);
	free(root);

	return 0;
}


void publish_positive_response_for_action(bytebeam_client *bb_obj,char *action_id)
{
	publish_action_status(bb_obj->device_cfg,action_id,100,bb_obj->client,"Completed","No Error");
}


void bytebeam_init(bytebeam_client *bb_obj)
{
	parse_device_config_file(&bb_obj->device_cfg, &bb_obj->mqtt_cfg);
	bytebeam_hal_init(bb_obj);
}


int publish_to_bytebeam_stream(bytebeam_client* bb_obj,char* stream_name,char* payload)
{
	int msg_id = 0;
	char topic[200] = {0,};
	sprintf(topic,"/tenants/%s/devices/%s/events/%s/jsonarray", bb_obj->device_cfg.project_id,bb_obj->device_cfg.device_id, stream_name);
	ESP_LOGI(TAG, "Topic is %s", topic);
	msg_id = esp_mqtt_client_publish(bb_obj->client, topic, (const char *)payload, strlen(payload), 1, 0);	
	if(connection_status == 1)
	{
		ESP_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id,payload);
	}
	else
	{
		ESP_LOGE(TAG,"Publish to %s stream Failed",stream_name);
	}
	return 0;			
}