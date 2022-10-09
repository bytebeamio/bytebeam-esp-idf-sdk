#ifndef BYTEBEAM_ESP_HAL_H
#define BYTEBEAM_ESP_HAL_H

int bytebeam_hal_mqtt_subscribe(void *client, char *topic, int qos);
int bytebeam_hal_mqtt_publish(void *client, char *topic, const char *message, int length, int qos);
int bytebeam_hal_restart(void);
int bytebeam_hal_ota(bytebeam_client* bb_obj, char *ota_url);
int bytebeam_hal_init(bytebeam_client *bb_obj);
int bytebeam_hal_start_mqtt(bytebeam_client *bb_obj);

#endif