idf_component_register(SRCS "app_main.c" )

fatfs_create_rawflash_image(storage ../config_data FLASH_IN_PROJECT)
