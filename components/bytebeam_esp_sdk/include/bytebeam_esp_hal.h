#ifndef BYTEBEAM_ESP_HAL_H
#define BYTEBEAM_ESP_HAL_H

int bytebeam_hal_mqtt_subscribe(void *client, char *topic, int qos);
int bytebeam_hal_mqtt_publish(void *client, char *topic, const char *message, int length, int qos);
int bytebeam_hal_restart(void);
int bytebeam_hal_ota(bytebeam_client_t *bytebeam_client, char *ota_url);
int bytebeam_hal_init(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_start_mqtt(bytebeam_client_t *bytebeam_client);

extern char *ota_action_id;

int bytebeam_subscribe_to_actions(bytebeam_device_config_t device_cfg, esp_mqtt_client_handle_t client);
int bytebeam_handle_actions(char *action_received, esp_mqtt_client_handle_t client, bytebeam_client_t *bytebeam_client);
int publish_action_status(bytebeam_device_config_t device_cfg, char *action_id, int percentage, bytebeam_client_handle_t client, char *status, char *error_message);

#endif