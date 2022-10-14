# Bytebeam ESP SDK
This SDK consist of components that can be used for connecting ESP devices to [Bytebeam](https://bytebeam.io/) IoT platform

## Features
- Efficiently send data to cloud.
- Receive commands from the cloud, execute them and update progress of execution.
- Download Firmware images from cloud in case of OTA updates being triggered from cloud.

## What's included in the SDK :-

- **components/bytebeam_esp_sdk** :-  This section contains source code for various functions that can be used by applications for interacting with Bytebeam platform. 
- **example** :- This folder conatins demo application which demonstrates establishing secure connection with Bytebeam platform. Also, it demonstrates periodic data pushing and receiving actions.
-**provisioning** :- This folder contains application for pushing device config data to SPIFFS of device.

## Dependencies :-
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) 
- ESP32 Board

## SDK Setup and Integration :-
This SDK can be integrated with new as well as existing ESP projects. Follow the [instruction guide](https://bytebeam.io/docs/esp-idf) for setting up and integrating SDK with your projects. 
