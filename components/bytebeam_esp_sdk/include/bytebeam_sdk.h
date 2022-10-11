#ifndef BYTEBEAM_SDK_H
#define BYTEBEAM_SDK_H

#include "mqtt_client.h"

#define NUMBER_OF_ACTIONS 10

typedef struct device_config {
    char *ca_cert_pem;
    char *client_cert_pem;
    char *client_key_pem;
    char broker_uri[100];
    char device_id[10];
    char project_id[100];
} device_config;

typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;

struct bytebeam_client_s;

typedef struct {
    const char *name;
    int (*func)(struct bytebeam_client_s *bb_obj, char *args, char *action_id);
} action_functions_map;

typedef struct bytebeam_client_s {
    device_config device_cfg;
    bytebeam_client_handle_t client;
    bytebeam_client_config_t mqtt_cfg;
    action_functions_map action_funcs[NUMBER_OF_ACTIONS];
    int connection_status;
} bytebeam_client;

extern char *ota_action_id;

int bytebeam_subscribe_to_actions(device_config device_cfg, esp_mqtt_client_handle_t client);
int bytebeam_handle_actions(char *action_received, esp_mqtt_client_handle_t client, bytebeam_client *bb_obj);
int bytebeam_init(bytebeam_client *bb_obj);
int bytebeam_publish_action_completed(bytebeam_client *bb_obj, char *action_id);
int bytebeam_publish_action_failed(bytebeam_client *bb_obj, char *action_id);
int bytebeam_publish_action_progress(bytebeam_client *bb_obj, char *action_id, int progress_percentage);
int bytebeam_publish_to_stream(bytebeam_client *bb_obj, char *stream_name, char *payload);
int bytebeam_start(bytebeam_client *bb_obj);
int publish_action_status(device_config device_cfg, char *action_id, int percentage, bytebeam_client_handle_t client, char *status, char *error_message);
int handle_ota(bytebeam_client *bb_obj, char *payload_string, char *action_id);
int bytebeam_create_new_action_handler(bytebeam_client *bb_obj, int (*func_ptr)(bytebeam_client *, char *, char *), char *func_name);
void bytebeam_init_action_handler_array(action_functions_map *action_handler_array);

#endif