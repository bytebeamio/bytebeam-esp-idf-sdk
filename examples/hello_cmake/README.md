| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- |

# Bytebeam Hello CMake Example
This example demonnstrates how to control the led gpio from Bytebeam IoT Platform.

## Hardware Required
- A development board with Espressif SoC (e.g.,ESP32-DevKitC, ESP-WROVER-KIT, etc.)
- A USB cable for Power supply and programming

## Software Required
- ESP-IDF
- Bytebeam ESP-IDF SDK

## Project Configurations

Open the project configuration menu (`config_app.bat`).

In the `Example Configuration` menu:

- Set the Wi-Fi configuration.
  - Set `WiFi SSID`.
  - Set `WiFi Password`.

Optional: If you need, change the other options according to your requirements.

To change `LED GPIO Configuration` head over to [led_driver.h](main/include/led_driver.h) file.

- Select the LED type in the Blink LED type option.
  -  Use GPIO for regular LED blink.
- Set the GPIO number used for the signal in the Blink GPIO number option.

## Build Flash Monitor

```
# build the binary for the target (say esp32)
$ build_app.bat esp32

# flash and monitor the app
$ flash_monitor.bat COM
```

(To exit the serial monitor, type Ctrl-].)

## Example Output

```
I (5697) BYTEBEAM_TOGGLE_LED_EXAMPLE: Initializing SNTP
I (5707) BYTEBEAM_TOGGLE_LED_EXAMPLE: Waiting for system time to be set... (1/10)
I (7717) BYTEBEAM_TOGGLE_LED_EXAMPLE: Waiting for system time to be set... (2/10)
I (9417) BYTEBEAM_TOGGLE_LED_EXAMPLE: Notification of a time synchronization event
W (9417) wifi:<ba-add>idx:1 (ifx:0, 5c:8c:30:c2:24:39), tid:1, ssn:0, winSize:64
I (9717) gpio: GPIO[2]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0
I (9717) BYTEBEAM_SDK: SPIFFS file system detected !
I (9737) BYTEBEAM_SDK: Reading file : /spiffs/device_config.json
I (9827) BYTEBEAM_SDK: The uri  is: mqtts://cloud.bytebeam.io:8883

I (9827) BYTEBEAM_SDK: [APP] Free memory: 225880 bytes
I (9837) BYTEBEAM_SDK: Normal reboot
I (9837) BYTEBEAM_SDK: Bytebeam Client Initialized !!
I (9837) BYTEBEAM_SDK: Other event id:7
I (9847) BYTEBEAM_SDK:
Status to send:
[{
                "timestamp":    1683887741980,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       9242,
                "Status":       "Device is Up!",
                "Software_Type":        "toggle-led-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]

I (9877) BYTEBEAM_SDK: Topic is /tenants/ota/devices/1/events/device_shadow/jsonarray
I (11977) BYTEBEAM_SDK: MQTT_EVENT_CONNECTED
I (11977) BYTEBEAM_SDK: MQTT SUBSCRIBED!! Msg ID:58285
I (11987) BYTEBEAM_SDK: sent publish successful, msg_id=10976, message:[{
                "timestamp":    1683887741980,
                "sequence":     1,
                "Reset_Reason": "Power On Reset",
                "Uptime":       9242,
                "Status":       "Device is Up!",
                "Software_Type":        "toggle-led-app",
                "Software_Version":     "1.0.0",
                "Hardware_Type":        "ESP32 DevKit V1",
                "Hardware_Version":     "rev1"
        }]
I (12007) BYTEBEAM_SDK: Bytebeam Client started !!
I (12217) BYTEBEAM_SDK: MQTT_EVENT_SUBSCRIBED, msg_id=58285
I (12307) BYTEBEAM_SDK: MQTT_EVENT_PUBLISHED, msg_id=10976
```

## Troubleshooting

- Ensure you've provisioned the device correctly
- Ensure working Wi-Fi connectivity

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.