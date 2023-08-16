#ifndef BYTEBEAM_HAL_H
#define BYTEBEAM_HAL_H

// define the sdk platform here
#define CONFIG_SDK_PLATFORM_ESP

#ifdef CONFIG_SDK_PLATFORM_ESP
#include "bytebeam_esp_hal.h"
#endif

#endif /* BYTEBEAM_HAL_H */