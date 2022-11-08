idf_component_register(SRCS "app_main.c" 
                       PRIV_REQUIRES "bytebeam_esp_sdk"
                                     "nvs_flash"
                                     "app_update"
                                     "mqtt"
                                     "protocol_examples_common"
                                     "esp_http_client"
                                     "esp_https_ota"
                                     "json"
                       INCLUDE_DIRS ".")
#Xtarget_add_binary_data(${CMAKE_PROJECT_NAME}.elf "main/device_1.json" TEXT)
