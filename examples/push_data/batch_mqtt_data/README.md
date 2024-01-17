| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Batch MQTT Data Example
This example demonnstrates how to batch mqtt data and send to Bytebeam IoT Platform.

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
I (5677) BYTEBEAM_BATCH_MQTT_DATA_EXAMPLE: Initializing SNTP
I (5687) BYTEBEAM_BATCH_MQTT_DATA_EXAMPLE: Waiting for system time to be set... (1/10)
I (6017) BYTEBEAM_BATCH_MQTT_DATA_EXAMPLE: Notification of a time synchronization event
I (7697) BYTEBEAM_CLIENT: SPIFFS file system detected !
I (7717) BYTEBEAM_CLIENT: Reading file : /spiffs/device_config.json
I (7807) BYTEBEAM_CLIENT: The uri  is: mqtts://cloud.bytebeam.io:8883

I (7807) BYTEBEAM_HAL: [APP] Free memory: 197696 bytes
I (7807) BYTEBEAM_HAL: Device contains factory Firmware

I (7807) BYTEBEAM_LOG: Setting Bytebeam Log Level : 3
I (7817) BYTEBEAM_CLIENT: Bytebeam Client Initialized !!
I (7827) BYTEBEAM_STREAM: Device Shadow Message.

I (7827) BYTEBEAM_HAL: Other event id:7
I (7837) BYTEBEAM_CLIENT: Bytebeam Client started !!
I (7847) BYTEBEAM_STREAM:
Status to send:
[{
        "timestamp":    1701513255270,
        "sequence":     1,
        "Reset_Reason": "Power On Reset",
        "Uptime":       7239,
        "Status":       "Device is Online",
        "Software_Type":        "batch-mqtt-data-app",
        "Software_Version":     "v0.1.0",
        "Hardware_Type":        "Bytebeam ESP32",
        "Hardware_Version":     "rev1"
}]

I (7867) wifi:<ba-add>idx:1 (ifx:0, 34:60:f9:05:a9:67), tid:3, ssn:0, winSize:64
I (7847) BYTEBEAM_STREAM: Mqtt Batch Size Now : 1

I (7877) BYTEBEAM_STREAM: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
I (8107) BYTEBEAM_STREAM: Mqtt Batch Size Now : 2

I (8307) BYTEBEAM_STREAM: Mqtt Batch Size Now : 3

I (8507) BYTEBEAM_STREAM: Mqtt Batch Size Now : 4

I (8707) BYTEBEAM_STREAM: Mqtt Batch Size Now : 5

I (8907) BYTEBEAM_STREAM: Mqtt Batch Size Now : 6

I (9107) BYTEBEAM_STREAM: Mqtt Batch Size Now : 7

I (9207) BYTEBEAM_HAL: MQTT_EVENT_CONNECTED
I (9217) BYTEBEAM_HAL: MQTT SUBSCRIBED!! Msg ID:39081
I (9217) BYTEBEAM_STREAM: sent publish successful, msg_id=16792
I (9237) BYTEBEAM_HAL: MQTT_EVENT_SUBSCRIBED, msg_id=39081
I (9257) BYTEBEAM_HAL: MQTT_EVENT_PUBLISHED, msg_id=16792
I (9307) BYTEBEAM_STREAM: Mqtt Batch Size Now : 8

I (9507) BYTEBEAM_STREAM: Mqtt Batch Size Now : 9

I (9707) BYTEBEAM_STREAM: Mqtt Batch Size Now : 10

I (9907) BYTEBEAM_STREAM: Mqtt Batch Size Now : 11

I (10107) BYTEBEAM_STREAM: Mqtt Batch Size Now : 12

I (10307) BYTEBEAM_STREAM: Mqtt Batch Size Now : 13

I (10507) BYTEBEAM_STREAM: Mqtt Batch Size Now : 14

I (10707) BYTEBEAM_STREAM: Mqtt Batch Size Now : 15

I (10907) BYTEBEAM_STREAM: Mqtt Batch Size Now : 16

I (11107) BYTEBEAM_STREAM: Mqtt Batch Size Now : 17

I (11307) BYTEBEAM_STREAM: Mqtt Batch Size Now : 18

I (11507) BYTEBEAM_STREAM: Mqtt Batch Size Now : 19

I (11707) BYTEBEAM_STREAM: Mqtt Batch Size Now : 20

I (11907) BYTEBEAM_STREAM: Mqtt Batch Size Now : 21

I (12107) BYTEBEAM_STREAM: Mqtt Batch Size Now : 22

I (12307) BYTEBEAM_STREAM: Mqtt Batch Size Now : 23

I (12507) BYTEBEAM_STREAM: Mqtt Batch Size Now : 24

I (12707) BYTEBEAM_STREAM: Mqtt Batch Size Now : 25
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.