# ESP_app
Application for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform

## Features
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.

## Dependencies

### ESP IDF installation

Refer here 

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

## How to use application

### Hardware Required

This example can be executed on any ESP32 board, the only required interface is WiFi and connection to internet.

### Configure the project

* Open the project configuration menu (`idf.py menuconfig`)
* Configure Wi-Fi or Ethernet under "Example Connection Configuration" menu. See "Establishing Wi-Fi or Ethernet Connection" section in [examples/protocols/README.md](../../README.md) for more details.
* When using Make build system, set `Default serial port` under `Serial flasher config`.

* Copy your device credentials file (device_XXXX.json) to the main directory

* edit main/component.mk file to include device_xxxx.json as below

````
COMPONENT_EMBED_TXTFILES := device_1222.json //use your device id in place of 1222
````
* edit CMakeList.txt to contain the following
```
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/device_1222.json" TEXT) //use your device id in place of 1222
```

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.
