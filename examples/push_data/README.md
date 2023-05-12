| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Push Data Example
This example demonnstrates how to push data to Bytebeam IoT Platform.

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
I (5673) BYTEBEAM_PUSH_DATA_EXAMPLE: Initializing SNTP
I (5683) BYTEBEAM_PUSH_DATA_EXAMPLE: Waiting for system time to be set... (1/10)
I (7693) BYTEBEAM_PUSH_DATA_EXAMPLE: Waiting for system time to be set... (2/10)
W (9263) wifi:I (9273) BYTEBEAM_PUSH_DATA_EXAMPLE: Notification of a time synchronization event
<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:5, ssn:0, winSize:64
I (9693) BYTEBEAM_SDK: SPIFFS file system detected !
I (9713) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (9803) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (9803) BYTEBEAM_SDK: [APP] Free memory: 225936 bytes
I (9803) BYTEBEAM_SDK: Normal reboot
I (9803) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (9813) BYTEBEAM_SDK: Other event id:7
I (9813) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683887056505,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       9218,
                "Status":       "Device is Up!",
                "Software_Type":        "push-data-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (9853) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
I (11543) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (11553) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:13944
I (11553) BYTEBEAM_SDK: sent publish successful, msg_id=64772, message:[{
                "timestamp":    1683887056505,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       9218,
                "Status":       "Device is Up!",
                "Software_Type":        "push-data-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (11573) BYTEBEAM_SDK: Bytebeam Client started !!
I (11583) BYTEBEAM_PUSH_DATA_EXAMPLE:
Status to send:
[{
                "timestamp":    1683887058275,
                "sequence":     1,
                "Status":       "Device Status Working !"
        }]

I (11593) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=13944
I (11603) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
I (11613) BYTEBEAM_SDK: sent publish successful, msg_id=56914, message:[{
                "timestamp":    1683887058275,
                "sequence":     1,
                "Status":       "Device Status Working !"
        }]
I (11633) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=64772
I (11683) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=56914
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.