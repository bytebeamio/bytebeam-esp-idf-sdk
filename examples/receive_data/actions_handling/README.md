| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Actions Handling Example
This example demonnstrates how to manage actions triggered from Bytebeam IoT Platform.

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
I (5674) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Initializing SNTP
I (5684) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Waiting for system time to be set... (1/10)
I (7694) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Waiting for system time to be set... (2/10)
I (9694) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Waiting for system time to be set... (3/10)
I (9784) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Notification of a time synchronization event
I (11694) BYTEBEAM_SDK: SPIFFS file system detected !
I (11714) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (11804) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (11804) BYTEBEAM_SDK: [APP] Free memory: 226224 bytes
I (11804) BYTEBEAM_SDK: Normal reboot
I (11804) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (11814) BYTEBEAM_SDK: Other event id:7
I (11814) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683884907742,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       11218,
                "Status":       "Device is Up!",
                "Software_Type":        "actions-handling-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (11844) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
W (11984) wifi:<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:3, ssn:0, winSize:64
I (14344) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (14344) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:32194
I (14354) BYTEBEAM_SDK: sent publish successful, msg_id=9592, message:[{
                "timestamp":    1683884907742,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       11218,
                "Status":       "Device is Up!",
                "Software_Type":        "actions-handling-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (14374) BYTEBEAM_SDK: Bytebeam Client started !!
I (14384) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Status : Connected
I (14384) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Project Id : ota, Device Id : 1
I (14904) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=32194
I (16024) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=9592
I (24394) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Status : Connected
I (24394) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Project Id : ota, Device Id : 1
I (34394) BYTEBEAM_SETUP_CLIENT_EXAMPLE: Status : Connected
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.