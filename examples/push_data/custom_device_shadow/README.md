| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Custom Device Shadow Example
This example demonnstrates how to push custom device shadow to Bytebeam IoT Platform.

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
I (5677) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Initializing SNTP
I (5687) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Waiting for system time to be set... (1/10)
I (7697) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Waiting for system time to be set... (2/10)
I (9697) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Waiting for system time to be set... (3/10)
I (9797) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Notification of a time synchronization event
I (11697) BYTEBEAM_CLIENT: SPIFFS file system detected !
I (11717) BYTEBEAM_CLIENT: Reading file : /spiffs/device_config.json
I (11807) BYTEBEAM_CLIENT: The uri  is: mqtts://cloud.bytebeam.io:8883

I (11807) BYTEBEAM_HAL: [APP] Free memory: 229332 bytes
I (11807) BYTEBEAM_HAL: Device contains factory Firmware

I (11817) BYTEBEAM_LOG: Setting Bytebeam Log Level : 3
I (11817) BYTEBEAM_CLIENT: Bytebeam Client Initialized !!
I (11827) BYTEBEAM_STREAM: Device Shadow Message.

I (11827) BYTEBEAM_HAL: Other event id:7
I (11837) BYTEBEAM_STREAM:
Status to send:
[{
        "timestamp":    1700308726706,
        "sequence":     1,
        "Reset_Reason": "Power On Reset",
        "Uptime":       11234,
        "Status":       "Device is Online",
        "Software_Type":        "bytebeam-app",
        "Software_Version":     "v0.1.0",
        "Hardware_Type":        "Bytebeam ESP32",
        "Hardware_Version":     "rev1"
},{
        "temperature":  12.229999542236328,
        "humidity":     90.1500015258789
}]

I (11867) BYTEBEAM_STREAM: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
I (11827) BYTEBEAM_CLIENT: Bytebeam Client started !!
I (13577) BYTEBEAM_HAL: MQTT_EVENT_CONNECTED
I (13587) BYTEBEAM_HAL: MQTT SUBSCRIBED!! Msg ID:20515
I (13587) BYTEBEAM_STREAM: sent publish successful, msg_id=5661, message:[{
        "timestamp":    1700308726706,
        "sequence":     1,
        "Reset_Reason": "Power On Reset",
        "Uptime":       11234,
        "Status":       "Device is Online",
        "Software_Type":        "bytebeam-app",
        "Software_Version":     "v0.1.0",
        "Hardware_Type":        "Bytebeam ESP32",
        "Hardware_Version":     "rev1"
},{
        "temperature":  12.229999542236328,
        "humidity":     90.1500015258789
}]
I (13627) BYTEBEAM_HAL: MQTT_EVENT_SUBSCRIBED, msg_id=20515
I (13777) BYTEBEAM_HAL: MQTT_EVENT_PUBLISHED, msg_id=5661
I (14107) wifi:<ba-add>idx:1 (ifx:0, 44:95:3b:2c:54:00), tid:5, ssn:12, winSize:64
I (21887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Status : Connected
I (21887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Project Id : ota, Device Id : 1
I (31887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Status : Connected
I (31887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Project Id : ota, Device Id : 1
I (41887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Status : Connected
I (41887) BYTEBEAM_CUSTOM_DEVICE_SHADOW_EXAMPLE: Project Id : ota, Device Id : 1
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.