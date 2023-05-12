| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Cloud Logging Example
This example demonnstrates how to log the useful information to Bytebeam IoT Platform.

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
I (5674) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Initializing SNTP
I (5684) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Waiting for system time to be set... (1/10)
I (6304) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Notification of a time synchronization event
I (7694) BYTEBEAM_SDK: SPIFFS file system detected !
I (7714) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (7804) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (7804) BYTEBEAM_SDK: [APP] Free memory: 226212 bytes
I (7804) BYTEBEAM_SDK: Normal reboot
I (7804) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (7814) BYTEBEAM_SDK: Other event id:7
I (7814) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683885926173,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       7219,
                "Status":       "Device is Up!",
                "Software_Type":        "cloud-logging-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (7844) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
W (7884) wifi:<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:3, ssn:1, winSize:64
I (9364) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (9374) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:37143
I (9374) BYTEBEAM_SDK: sent publish successful, msg_id=5985, message:[{
                "timestamp":    1683885926173,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       7219,
                "Status":       "Device is Up!",
                "Software_Type":        "cloud-logging-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (9404) BYTEBEAM_SDK: Bytebeam Client started !!
I (9404) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Cloud Logging is Enabled.
I (9414) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=37143
I (9414) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/logs/jsonarray
I (9424) BYTEBEAM_SDK: sent publish successful, msg_id=62988, message:[{
                "timestamp":    1683885927774,
                "sequence":     1,
                "level":        "Info",
                "tag":  "BYTEBEAM_CLOUD_LOGGING_EXAMPLE",
                "message":      "Status : Connected"
        }]
I (9444) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Status : Connected
I (9454) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=5985
I (9454) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/logs/jsonarray
I (9474) BYTEBEAM_SDK: sent publish successful, msg_id=36305, message:[{
                "timestamp":    1683885927814,
                "sequence":     2,
                "level":        "Info",
                "tag":  "BYTEBEAM_CLOUD_LOGGING_EXAMPLE",
                "message":      "Project Id : ota, Device Id : 1"
        }]
I (9484) BYTEBEAM_CLOUD_LOGGING_EXAMPLE: Project Id : ota, Device Id : 1
I (9494) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=62988
I (9534) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=36305
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.