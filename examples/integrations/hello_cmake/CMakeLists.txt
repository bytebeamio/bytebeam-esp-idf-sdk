cmake_minimum_required(VERSION 3.16)

project(hello_cmake C)

set(targets "esp32" "esp32s2" "esp32s3" "esp32c3" "esp32c2" "esp32c6" "esp32h2" "esp32p4")

if("${TARGET}" IN_LIST targets)
    # Include for ESP-IDF build system functions
    include($ENV{IDF_PATH}/tools/cmake/idf.cmake)

    # Create protocol examples common component static library
    idf_build_component($ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

    # Create bytebeam esp idf sdk component static library
    idf_build_component(components/led_strip)

    # Create bytebeam esp idf sdk component static library
    idf_build_component(../../../)

    # Create idf::{target} and idf::freertos static libraries
    idf_build_process("${TARGET}"
                    # try and trim the build; additional components
                    # will be included as needed based on dependency tree
                    #
                    # although esptool_py does not generate static library,
                    # processing the component is needed for flashing related
                    # targets and file generation
                    COMPONENTS freertos esptool_py json mqtt esp_wifi driver nvs_flash app_update
                               console protocol_examples_common esp_http_client esp_https_ota bytebeam-esp-idf-sdk
                               led_strip
                    SDKCONFIG ${CMAKE_CURRENT_LIST_DIR}/sdkconfig
                    SDKCONFIG_DEFAULTS ${CMAKE_CURRENT_LIST_DIR}/sdkconfig.defaults
                    BUILD_DIR ${CMAKE_BINARY_DIR})
else()
    message(WARNING "Unknown target ${TARGET}, creating stubs for esp32 instead")
    # Create stubs for esp32 and freertos, stub::esp32 and stub::freertos
    add_subdirectory(stubs/esp32)
    add_subdirectory(stubs/freertos)
    add_subdirectory(stubs/spi_flash)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(elf_file ${CMAKE_PROJECT_NAME}.elf)
include_directories(main/include)
add_executable(${elf_file} main/app_main.c main/led_driver.c)

# Link the static libraries to the executable
if("${TARGET}" IN_LIST targets)
    target_link_libraries(${elf_file} idf::freertos idf::spi_flash idf::json idf::mqtt
                                      idf::esp_wifi idf::driver idf::nvs_flash idf::app_update
                                      idf::esp_http_client idf::esp_https_ota idf::console
                                      idf::protocol_examples_common idf::bytebeam-esp-idf-sdk
                                      idf::led_strip)
    # Attach additional targets to the executable file for flashing,
    # linker script generation, partition_table generation, etc.
    idf_build_executable(${elf_file})
else()
    target_link_libraries(${elf_file} stub::esp32 stub::freertos stub::spi_flash)
endif()