#include "cJSON.h"

extern int connection_status;

int bytebeam_hal_mqtt_subscribe(void *client, char *topic, int qos);
int bytebeam_hal_mqtt_publish(void *client, char *topic, const char *message, int length, int qos);
int bytebeam_hal_restart(void *);
int bytebeam_hal_ota(void *input, char *ota_url, bytebeam_client_handle_t test_handle);
int bytebeam_hal_init(bytebeam_client *bb_obj);