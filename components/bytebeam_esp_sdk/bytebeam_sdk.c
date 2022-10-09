#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_log.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "bytebeam_sdk.h"
#include "bytebeam_esp_hal.h"
#include "nvs.h"
#include "nvs_flash.h"

char *ota_action_id = "";

const uint8_t device_cert_json_start[] asm("_binary_device_1_json_start");

static const char *TAG = "BYTEBEAM_SDK";

int bytebeam_subscribe_to_actions(device_config device_cfg, bytebeam_client_handle_t client)
{
	int msg_id;
	char topic[200] = {
		0,
	};

	sprintf(topic, "/tenants/%s/devices/%s/actions", device_cfg.project_id, device_cfg.device_id);
	msg_id = bytebeam_hal_mqtt_subscribe(client, topic, 1);

	return msg_id;
}

int parse_device_config_file(device_config *device_cfg, esp_mqtt_client_config_t *mqtt_cfg)
{
	const char *json_file = (const char *)device_cert_json_start;

	cJSON *cert_json = cJSON_Parse(json_file);

	if (cert_json == NULL)
	{
		ESP_LOGE(TAG, "ERROR in parsing the JSON\n");
		return -1;
	}

	cJSON *prj_id_obj = cJSON_GetObjectItem(cert_json, "project_id");

	if (!(cJSON_IsString(prj_id_obj) && (prj_id_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR in getting the prject id\n");
		return -1;
	}

	strcpy(device_cfg->project_id, prj_id_obj->valuestring);

	cJSON *broker_name_obj = cJSON_GetObjectItem(cert_json, "broker");

	if (!(cJSON_IsString(broker_name_obj) && (broker_name_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing broker name");
		return -1;
	}

	cJSON *port_num_obj = cJSON_GetObjectItem(cert_json, "port");

	if (!(cJSON_IsNumber(port_num_obj)))
	{
		ESP_LOGE(TAG, "ERROR parsing port number.");
		return -1;
	}

	int port_int = port_num_obj->valuedouble;
	sprintf(device_cfg->broker_uri, "mqtts://%s:%d", broker_name_obj->valuestring, port_int);
	mqtt_cfg->uri = device_cfg->broker_uri;

	ESP_LOGI(TAG, "The uri  is: %s\n", mqtt_cfg->uri);

	cJSON *device_id_obj = cJSON_GetObjectItem(cert_json, "device_id");

	if (!(cJSON_IsString(device_id_obj) && (device_id_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing device id\n");
		return -1;
	}

	strcpy(device_cfg->device_id, device_id_obj->valuestring);
	cJSON *auth_obj = cJSON_GetObjectItem(cert_json, "authentication");

	if (cert_json == NULL || !(cJSON_IsObject(auth_obj)))
	{
		ESP_LOGE(TAG, "ERROR in parsing the auth JSON\n");
		return -1;
	}

	cJSON *ca_cert_obj = cJSON_GetObjectItem(auth_obj, "ca_certificate");

	if (!(cJSON_IsString(ca_cert_obj) && (ca_cert_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing ca certificate\n");
		return -1;
	}

	device_cfg->ca_cert_pem = (char *)ca_cert_obj->valuestring;
	mqtt_cfg->cert_pem = device_cfg->ca_cert_pem;

	cJSON *device_cert_obj = cJSON_GetObjectItem(auth_obj, "device_certificate");

	if (!(cJSON_IsString(device_cert_obj) && (device_cert_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR parsing device certifate\n");
		return -1;
	}

	device_cfg->client_cert_pem = (char *)device_cert_obj->valuestring;
	mqtt_cfg->client_cert_pem = device_cfg->client_cert_pem;

	cJSON *device_private_key_obj = cJSON_GetObjectItem(auth_obj, "device_private_key");

	if (!(cJSON_IsString(device_private_key_obj) && (device_private_key_obj->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "ERROR device private key\n");
		return -1;
	}

	device_cfg->client_key_pem = (char *)device_private_key_obj->valuestring;
	mqtt_cfg->client_key_pem = (char *)device_cfg->client_key_pem;
	return 0;
}

int bytebeam_handle_actions(char *action_received, bytebeam_client_handle_t client, bytebeam_client *bb_obj)
{
	cJSON *root = NULL;
	cJSON *name = NULL;
	cJSON *payload = NULL;
	cJSON *action_id_obj = NULL;

	int action_iterator = 0;
	char action_id[20] = {
		0,
	};

	root = cJSON_Parse(action_received);

	name = cJSON_GetObjectItem(root, "name");

	if (!(cJSON_IsString(name) && (name->valuestring != NULL)))
	{
		ESP_LOGE(TAG, "Error parsing action name\n");
		goto cleanup;
		return -1;
	}

	ESP_LOGI(TAG, "Checking name \"%s\"\n", name->valuestring);

	action_id_obj = cJSON_GetObjectItem(root, "id");
	strcpy(action_id, action_id_obj->valuestring);

	if (cJSON_IsString(action_id_obj) && (action_id_obj->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking version \"%s\"\n", action_id_obj->valuestring);
	}
	else
	{
		ESP_LOGE(TAG, "Error parsing action id");

		goto cleanup;
		return -1;
	}

	payload = cJSON_GetObjectItem(root, "payload");

	if (cJSON_IsString(payload) && (payload->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking payload \"%s\"\n", payload->valuestring);

		while (bb_obj->action_funcs[action_iterator].name)
		{
			if (!strcmp(bb_obj->action_funcs[action_iterator].name, name->valuestring))
			{
				bb_obj->action_funcs[action_iterator].func(bb_obj, payload->valuestring, action_id);
				break;
			}

			action_iterator++;
		}

		if (bb_obj->action_funcs[action_iterator].name == NULL)
		{
			ESP_LOGI(TAG, "Invalid action:%s\n", name->valuestring);
		}
	}
	else
	{
		ESP_LOGE(TAG, "Error fetching payload");

		goto cleanup;
		return -1;
	}

	goto cleanup;

cleanup:
	free(payload);
	free(name);
	free(root);

	return 0;
}

int bytebeam_publish_action_completed(bytebeam_client *bb_obj, char *action_id)
{
	int ret_val = 0;

	ret_val = publish_action_status(bb_obj->device_cfg, action_id, 100, bb_obj->client, "Completed", "No Error");

	if (ret_val != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int bytebeam_publish_action_failed(bytebeam_client *bb_obj, char *action_id)
{
	int ret_val = 0;

	ret_val = publish_action_status(bb_obj->device_cfg, action_id, 0, bb_obj->client, "Failed", "Action failed");

	if (ret_val != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int bytebeam_publish_action_progress(bytebeam_client *bb_obj, char *action_id, int progress_percentage)
{
	int ret_val = 0;

	ret_val = publish_action_status(bb_obj->device_cfg, action_id, progress_percentage, bb_obj->client, "Progress", "No Error");

	if (ret_val != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int bytebeam_init(bytebeam_client *bb_obj)
{
	int ret_val = 0;
	ret_val = parse_device_config_file(&bb_obj->device_cfg, &bb_obj->mqtt_cfg);

	if (ret_val != 0)
	{
		ESP_LOGE(TAG, "MQTT init failed due to error in parsing device config JSON");
		return -1;
	}

	if ((bytebeam_hal_init(bb_obj)) != 0)
	{
		return -1;
	}

	return 0;
}

int bytebeam_start(bytebeam_client *bb_obj)
{
	if ((bytebeam_hal_start_mqtt(bb_obj) != 0))
	{
		ESP_LOGE(TAG, "MQTT Client start failed");
		return -1;
	}
	else
	{
		return 0;
	}
}

int bytebeam_publish_to_stream(bytebeam_client *bb_obj, char *stream_name, char *payload)
{
	int msg_id = 0;
	char topic[200] = {
		0,
	};

	sprintf(topic, "/tenants/%s/devices/%s/events/%s/jsonarray", bb_obj->device_cfg.project_id, bb_obj->device_cfg.device_id, stream_name);
	ESP_LOGI(TAG, "Topic is %s", topic);

	msg_id = esp_mqtt_client_publish(bb_obj->client, topic, (const char *)payload, strlen(payload), 1, 0);

	if (bb_obj->connection_status == 1)
	{
		ESP_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id, payload);
	}
	else
	{
		ESP_LOGE(TAG, "Publish to %s stream Failed", stream_name);
	}

	return msg_id;
}

int parse_ota_json(char *payload_string, char *url_string_return)
{
	const cJSON *url = NULL;
	const cJSON *version = NULL;

	cJSON *pl_json = cJSON_Parse(payload_string);
	url = cJSON_GetObjectItem(pl_json, "url");

	if (cJSON_IsString(url) && (url->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking url \"%s\"\n", url->valuestring);
	}
	else
	{
		ESP_LOGE(TAG, "URL parsing failed");
		return -1;
	}

	version = cJSON_GetObjectItem(pl_json, "version");

	if (cJSON_IsString(version) && (version->valuestring != NULL))
	{
		ESP_LOGI(TAG, "Checking version \"%s\"\n", version->valuestring);
	}
	else
	{
		ESP_LOGE(TAG, "FW version parsing failed");
		return -1;
	}

	sprintf(url_string_return, "%s", url->valuestring);
	ESP_LOGI(TAG, "The constructed URL is: %s", url_string_return);

	return 0;
}

int publish_action_status(device_config device_cfg, char *action_id, int percentage, bytebeam_client_handle_t client, char *status, char *error_message)
{
	static uint64_t sq_num = 0;
	cJSON *action_status_json_list = NULL;
	cJSON *action_status_json = NULL;
	cJSON *percentage_json = NULL;
	cJSON *timestamp_json = NULL;
	cJSON *device_status_json = NULL;
	cJSON *action_id_json = NULL;
	cJSON *action_errors_json = NULL;
	cJSON *seq_json = NULL;
	char *string_json = NULL;

	const char *ota_states[1] = {
		"Dummy_data"};

	action_status_json_list = cJSON_CreateArray();

	if (action_status_json_list == NULL)
	{
		ESP_LOGE(TAG, "Json Init failed.");
		goto END;
		return -1;
	}

	action_status_json = cJSON_CreateObject();

	if (action_status_json == NULL)
	{
		ESP_LOGE(TAG, "Json add failed.");
		goto END;
		return -1;
	}

	struct timeval te;
	sq_num++;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

	timestamp_json = cJSON_CreateNumber(milliseconds);

	if (timestamp_json == NULL)
	{
		ESP_LOGE(TAG, "Json add time stamp failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "timestamp", timestamp_json);

	seq_json = cJSON_CreateNumber(sq_num);

	if (seq_json == NULL)
	{
		ESP_LOGE(TAG, "Json add seq id failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "sequence", seq_json);

	device_status_json = cJSON_CreateString(status);

	if (device_status_json == NULL)
	{
		ESP_LOGE(TAG, "Json add device status failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "state", device_status_json);

	ota_states[0] = error_message;
	action_errors_json = cJSON_CreateStringArray(ota_states, 1);

	if (action_errors_json == NULL)
	{
		ESP_LOGE(TAG, "Json add action errors failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "errors", action_errors_json);

	action_id_json = cJSON_CreateString(action_id);

	if (action_id_json == NULL)
	{
		ESP_LOGE(TAG, "Json add action_id failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "id", action_id_json);

	percentage_json = cJSON_CreateNumber(percentage);

	if (percentage_json == NULL)
	{
		ESP_LOGE(TAG, "Json add progress percentage failed.");
		goto END;
		return -1;
	}

	cJSON_AddItemToObject(action_status_json, "progress", percentage_json);

	cJSON_AddItemToArray(action_status_json_list, action_status_json);

	string_json = cJSON_Print(action_status_json_list);
	ESP_LOGI(TAG, "\nTrying to print:\n%s\n", string_json);

	char topic[300] = {
		0,
	};

	sprintf(topic, "/tenants/%s/devices/%s/action/status", device_cfg.project_id, device_cfg.device_id);
	ESP_LOGI(TAG, "\n%s\n", topic);

	int msg_id = bytebeam_hal_mqtt_publish(client, topic, string_json, strlen(string_json), 1);

	if (msg_id != -1)
	{
		ESP_LOGI(TAG, "sent publish successful, msg_id=%d, message:%s", msg_id, string_json);
	}
	else
	{
		ESP_LOGE(TAG, "Publish Failed");
		return -1;
	}

END:
	cJSON_Delete(action_status_json_list);
	free(string_json);

	return 0;
}

int perform_ota(bytebeam_client *bb_obj, char *action_id, char *ota_url)
{
	// test_device_config = bb_obj->device_cfg;
	ESP_LOGI(TAG, "Starting OTA.....");

	if ((bytebeam_hal_ota(bb_obj, ota_url)) != -1)
	{
		nvs_handle_t nvs_handle;
		int32_t update_flag = 1;
		int32_t action_id_val = (int32_t)(atoi(ota_action_id));
		nvs_flash_init();
		nvs_open("test_storage", NVS_READWRITE, &nvs_handle);
		nvs_set_i32(nvs_handle, "update_flag", update_flag);
		nvs_commit(nvs_handle);
		nvs_set_i32(nvs_handle, "action_id_val", action_id_val);
		nvs_commit(nvs_handle);
		nvs_close(nvs_handle);
		bytebeam_hal_restart();
	}
	else
	{
		ESP_LOGE(TAG, "Firmware Upgrade Failed");

		if ((bytebeam_publish_action_failed(bb_obj, action_id)) != 0)
		{
			ESP_LOGE(TAG, "Failed to publish negative response for Firmware upgrade failure");
		}

		return -1;
	}

	return 0;
}

int handle_ota(bytebeam_client *bb_obj, char *payload_string, char *action_id)
{
	char constrcuted_url[200] = {
		0,
	};

	if ((parse_ota_json(payload_string, constrcuted_url)) == -1)
	{
		ESP_LOGE(TAG, "Firmware upgrade failed due to error in parsing OTA JSON");
		return -1;
	}

	ota_action_id = action_id;

	if ((perform_ota(bb_obj, action_id, constrcuted_url)) == -1)
	{
		return -1;
	}

	return 0;
}

int bytebeam_create_new_action_handler(bytebeam_client *bb_obj, int (*func_ptr)(bytebeam_client *, char *, char *), char *func_name)
{
	static int function_handler_index = 0;

	if (function_handler_index >= NUMBER_OF_ACTIONS)
	{
		ESP_LOGE(TAG, "Creation of new action handler failed");
		return -1;
	}

	bb_obj->action_funcs[function_handler_index].func = func_ptr;
	bb_obj->action_funcs[function_handler_index].name = func_name;

	function_handler_index = function_handler_index + 1;

	return 0;
}

void bytebeam_init_action_handler_array(action_functions_map *action_handler_array)
{
	int loop_var = 0;

	for (loop_var = 0; loop_var < NUMBER_OF_ACTIONS; loop_var++)
	{
		action_handler_array[loop_var].func = NULL;
		action_handler_array[loop_var].name = NULL;
	}
}