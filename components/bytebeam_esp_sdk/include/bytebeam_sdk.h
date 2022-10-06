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
} device_config;

typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;

typedef struct bytebeam_client
{
	device_config device_cfg;
	bytebeam_client_handle_t client;
	bytebeam_client_config_t mqtt_cfg;
} bytebeam_client;

extern int ota_update_completed;
extern char ota_action_id_str[15];

int bytebeam_subscribe_to_actions(device_config device_cfg, esp_mqtt_client_handle_t client);
int bytebeam_handle_actions(char *action_received, esp_mqtt_client_handle_t client, bytebeam_client *bb_obj);
void bytebeam_init(bytebeam_client *bb_obj);
void bytebeam_publish_action_completed(bytebeam_client *bb_obj, char *action_id);
void bytebeam_publish_action_failed(bytebeam_client *bb_obj, char *action_id);
void bytebeam_publish_action_progress(bytebeam_client *bb_obj, char *action_id, int progress_percentage);
int bytebeam_publish_to_stream(bytebeam_client *bb_obj, char *stream_name, char *payload);
void bytebeam_create_new_action_handler(int (*func_ptr)(bytebeam_client *, char *, char *), char *func_name);
