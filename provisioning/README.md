## Provisioning

Before you get started with the **bytebeam-esp-idf-sdk** you need to provision your device. Each device have it's own config file which needs to be flased into it's file system. As long as the file system is supported by the device, you can pick any of the following file systems for device provisioning.

- spiffs_provisioning
- fatfs_provisioning

Default Configuration,

| Arch          | File Sytem    | File Name           |
| ------------- |---------------| --------------------|
| ESP32         | SPIFFS        | device_config.json  |
 
You can always override the default configurations and here are steps to do so,
 - Provision your device with the required configurations
 - Specify the same configurations in your example app via `idf.py menuconfig`.

See [Advance SDK Configurations]() for the implementation details.