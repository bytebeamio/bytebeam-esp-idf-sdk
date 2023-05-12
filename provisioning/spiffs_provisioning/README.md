# Spiffs Provisioning Example
This example will place the device config file into the spiffs partition.

## Getting Device Config File
To get the device config file refer to the [Provisioning a Device](https://bytebeam.io/docs/provisioning-a-device) guide.

## Providing Device Config File
Once you get the device config file, place it inside the `config_data` directory with name as `device_config.json`.

## Hardware Required
- A development board with Espressif SoC (e.g.,ESP32-DevKitC, ESP-WROVER-KIT, etc.)
- A USB cable for Power supply and programming

## Software Required
- ESP-IDF
- Bytebeam ESP-IDF SDK

## Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type Ctrl-].)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
I (0) cpu_start: App cpu up.
I (260) cpu_start: Pro cpu start user code
I (260) cpu_start: cpu freq: 160000000 Hz
I (260) cpu_start: Application information:
I (265) cpu_start: Project name:     bytebeam_provisioning_example
I (272) cpu_start: App version:      a69d871-dirty
I (278) cpu_start: Compile time:     May 12 2023 19:22:41
I (284) cpu_start: ELF file SHA256:  a4d022e06a96599c...
I (290) cpu_start: ESP-IDF:          v5.0-dirty
I (295) heap_init: Initializing. RAM available for dynamic allocation:
I (302) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (308) heap_init: At 3FFB2F18 len 0002D0E8 (180 KiB): DRAM
I (314) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (321) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (327) heap_init: At 4008BFE4 len 0001401C (80 KiB): IRAM
I (335) spi_flash: detected chip: generic
I (338) spi_flash: flash io: dio
I (343) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (408) BYTEBEAM_PROVISIONING_EXAMPLE: Reading file : /spiffs/device_config.json
I (478) BYTEBEAM_PROVISIONING_EXAMPLE: Device Provisioning Success.
```

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/bytebeamio/bytebeam-esp-idf-sdk/issues) on GitHub. We will get back to you soon.