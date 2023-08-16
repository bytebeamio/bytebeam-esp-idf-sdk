#ifndef BYTEBEAM_ESP_HAL_H
#define BYTEBEAM_ESP_HAL_H

#include "esp_log.h"
#include "bytebeam_client.h"

typedef enum bytebeam_reset_reason {
    BB_RST_UNKNOWN,    //!< Reset reason can not be determined
    BB_RST_POWERON,    //!< Reset due to power-on event
    BB_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    BB_RST_SW,         //!< Software reset via esp_restart
    BB_RST_PANIC,      //!< Software reset due to exception/panic
    BB_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    BB_RST_TASK_WDT,   //!< Reset due to task watchdog
    BB_RST_WDT,        //!< Reset due to other watchdogs
    BB_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    BB_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    BB_RST_SDIO,       //!< Reset over SDIO
} bytebeam_reset_reason_t;

#define BB_LOGE(tag, fmt, ...)  ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#define BB_LOGW(tag, fmt, ...)  ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define BB_LOGI(tag, fmt, ...)  ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define BB_LOGD(tag, fmt, ...)  ESP_LOGD(tag, fmt, ##__VA_ARGS__)
#define BB_LOGV(tag, fmt, ...)  ESP_LOGV(tag, fmt, ##__VA_ARGS__)

int bytebeam_hal_mqtt_subscribe(bytebeam_client_handle_t client, char *topic, int qos);
int bytebeam_hal_mqtt_unsubscribe(bytebeam_client_handle_t client, char *topic);
int bytebeam_hal_mqtt_publish(bytebeam_client_handle_t client, char *topic, char *message, int length, int qos);
int bytebeam_hal_restart(void);
int bytebeam_hal_ota(bytebeam_client_t *bytebeam_client, char *ota_url);
int bytebeam_hal_init(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_destroy(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_start_mqtt(bytebeam_client_t *bytebeam_client);
int bytebeam_hal_stop_mqtt(bytebeam_client_t *bytebeam_client);

int bytebeam_hal_spiffs_mount();
int bytebeam_hal_spiffs_unmount();
int bytebeam_hal_fatfs_mount();
int bytebeam_hal_fatfs_unmount();
unsigned long long bytebeam_hal_get_epoch_millis();
bytebeam_reset_reason_t bytebeam_hal_get_reset_reason();
long long bytebeam_hal_get_uptime_ms();

int bytebeam_subscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client);
int bytebeam_unsubscribe_to_actions(bytebeam_device_config_t device_cfg, bytebeam_client_handle_t client);
int bytebeam_handle_actions(char *action_received, bytebeam_client_handle_t client, bytebeam_client_t *bytebeam_client);
int bytebeam_publish_device_heartbeat(bytebeam_client_t *bytebeam_client);

extern char *ota_action_id;
extern char ota_error_str[];

#endif /* BYTEBEAM_ESP_HAL_H */