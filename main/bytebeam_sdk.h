#include "mqtt_client.h"
#include "cJSON.h"

typedef struct device_config
{
	char *ca_cert_pem;
	char *client_cert_pem;
	char *client_key_pem;
	char broker_uri[100];
	char device_id[10];
	char project_id[100];
}device_config;

typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;
typedef struct bytebeam_client
{
	device_config device_cfg;
	bytebeam_client_handle_t client;
	bytebeam_client_config_t mqtt_cfg;
}bytebeam_client;

int subscribe_to_actions(device_config device_cfg, esp_mqtt_client_handle_t client);
int handle_actions(char* action_received, esp_mqtt_client_handle_t client, bytebeam_client *bb_obj);
int publish_to_stream(esp_mqtt_client_handle_t client, device_config device_cfg, char* stream_name, char* json_to_publish);
void bytebeam_init(bytebeam_client *bb_obj);

