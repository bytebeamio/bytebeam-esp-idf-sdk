idf_component_register(
    INCLUDE_DIRS 
        "."
        "sht"
    SRCS 
        "app_main.c"
        "sht/sht31.c"    
    PRIV_REQUIRES
        "json"
        "mqtt"
        "esp_wifi"	
        "driver"
        "nvs_flash"
        "app_update"
        "protocol_examples_common"
        "esp_http_client"
        "esp_https_ota")
