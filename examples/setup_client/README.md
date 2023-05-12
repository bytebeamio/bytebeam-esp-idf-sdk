| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Setup Client Example
This example demonnstrates how to setup the client for Bytebeam IoT Platform.

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
I (5673) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Initializing SNTP
I (5683) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Waiting for system time to be set... (1/10)
I (7673) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Notification of a time synchronization event
I (7693) BYTEBEAM_SDK: SPIFFS file system detected !
I (7713) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (7803) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (7803) BYTEBEAM_SDK: [APP] Free memory: 226228 bytes
I (7803) BYTEBEAM_SDK: Normal reboot
I (7803) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (7813) BYTEBEAM_SDK: Other event id:7
I (7813) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683887369738,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       7217,
                "Status":       "Device is Up!",
                "Software_Type":        "setup-client-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (7843) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
W (8043) wifi:<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:3, ssn:0, winSize:64
I (10033) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (10043) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:54213
I (10053) BYTEBEAM_SDK: sent publish successful, msg_id=20343, message:[{
                "timestamp":    1683887369738,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       7217,
                "Status":       "Device is Up!",
                "Software_Type":        "setup-client-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (10073) BYTEBEAM_SDK: Bytebeam Client started !!
I (10073) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Status : Connected
I (10083) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Project Id : ota, Device Id : 1
I (10203) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=54213
I (10613) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=20343
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.