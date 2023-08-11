#ifndef BYTEBEAM_ESP_HAL_H
#define BYTEBEAM_ESP_HAL_H

int bytebeam_hal_mqtt_subscribe(bytebeam_client_handle_t client, char *topic, int qos);
int bytebeam_hal_mqtt_unsubscribe(bytebeam_client_handle_t client, char *topic);
int bytebeam_hal_mqtt_publish(bytebeam_client_handle_t client, char *topic, char *message, int length, int qos);
int bytebeam_hal_restart(void);
int bytebeam_hal_ota(bytebeam_client_t *bytebeam_client, char *ota_url);
int bytebeam_hal_init(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_destroy(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_start_mqtt(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_stop_mqtt(bytebeam_client_t *bytebeam_client);

int bytebeam_subscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client);
int bytebeam_unsubscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client);
int bytebeam_handle_actions(char *action_received, bytebeam_client_handle_t client, bytebeam_client_t *bytebeam_client);
int bytebeam_publish_device_heartbeat(bytebeam_client_t *bytebeam_client);
int bytebeam_publish_device_coredump(bytebeam_client_t *bytebeam_client);

extern char *ota_action_id;
extern char ota_error_str[];

#endif /* BYTEBEAM_ESP_HAL_H */