| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Basic OTA With Rollback Example
This example demonnstrates how to rollback over the air firmware upgrades from Bytebeam IoT Platform.

## Hardware Required
- A development board with Espressif SoC (e.g.,ESP32-DevKitC, ESP-WROVER-KIT, etc.)
- A USB cable for Power supply and programming

## Software Required
- ESP-IDF
- Bytebeam ESP-IDF SDK

## Project Configurations

Open the project configuration menu (`idf.py menuconfig`).

In the `Example Configuration` menu:

- Set the Wi-Fi configuration.
  - Set `WiFi SSID`.
  - Set `WiFi Password`.

Optional: If you need, change the other options according to your requirements.

## Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type Ctrl-].)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
I (5690) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Initializing SNTP
I (5700) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Waiting for system time to be set... (1/10)
I (7710) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Waiting for system time to be set... (2/10)
I (9710) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Waiting for system time to be set... (3/10)
I (9870) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Notification of a time synchronization event
I (11710) BYTEBEAM_SDK: SPIFFS file system detected !
I (11730) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (11820) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (11820) BYTEBEAM_SDK: [APP] Free memory: 226004 bytes
I (11820) BYTEBEAM_SDK: Normal reboot
I (11820) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (11830) BYTEBEAM_SDK: Other event id:7
I (11830) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683885502755,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       11218,
                "Status":       "Device is Up!",
                "Software_Type":        "basic-ota-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (11860) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
W (11900) wifi:<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:3, ssn:1, winSize:64
I (14010) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (14010) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:35222
I (14020) BYTEBEAM_SDK: sent publish successful, msg_id=7659, message:[{
                "timestamp":    1683885502755,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       11218,
                "Status":       "Device is Up!",
                "Software_Type":        "basic-ota-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (14040) BYTEBEAM_SDK: Bytebeam Client started !!
I (14040) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Application Firmware Version : 1.0.0
I (14050) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=35222
I (14060) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Status : Connected
I (14070) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Project Id : ota, Device Id : 1
I (14090) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=7659
I (24080) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Status : Connected
I (24080) BYTEBEAM_BASIC_OTA_WITH_ROLLBACK_EXAMPLE: Project Id : ota, Device Id : 1
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.