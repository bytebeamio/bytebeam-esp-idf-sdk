#ifndef BYTEBEAM_SDK_H
#define BYTEBEAM_SDK_H

#include "mqtt_client.h"

#define BYTEBEAM_NUMBER_OF_ACTIONS 10

typedef struct bytebeam_device_config_t {
    char *ca_cert_pem;
    char *client_cert_pem;
    char *client_key_pem;
    char broker_uri[100];
    char device_id[10];
    char project_id[100];
} bytebeam_device_config_t;

typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;

struct bytebeam_client;

typedef struct {
    const char *name;
    int (*func)(struct bytebeam_client *bb_obj, char *args, char *action_id);
} bytebeam_action_functions_map_t;

typedef struct bytebeam_client {
    bytebeam_device_config_t device_cfg;
    bytebeam_client_handle_t client;
    bytebeam_client_config_t mqtt_cfg;
    bytebeam_action_functions_map_t action_funcs[BYTEBEAM_NUMBER_OF_ACTIONS];
    int connection_status;
} bytebeam_client_t;

int bytebeam_init(bytebeam_client_t *bb_obj);

int bytebeam_publish_action_completed(bytebeam_client_t *bb_obj, char *action_id);
int bytebeam_publish_action_failed(bytebeam_client_t *bb_obj, char *action_id);
int bytebeam_publish_action_progress(bytebeam_client_t *bb_obj, char *action_id, int progress_percentage);

int bytebeam_publish_to_stream(bytebeam_client_t *bb_obj, char *stream_name, char *payload);
int bytebeam_start(bytebeam_client_t *bb_obj);
int bytebeam_add_action_handler(bytebeam_client_t *bb_obj, int (*func_ptr)(bytebeam_client_t *, char *, char *), char *func_name);

int handle_ota(bytebeam_client_t *bb_obj, char *payload_string, char *action_id);

#endif