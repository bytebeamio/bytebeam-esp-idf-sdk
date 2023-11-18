#pragma once

// driver configurations
#define CONFIG_BLINK_GPIO 5
#define CONFIG_BLINK_LED_GPIO 1

// this macro is used to specify the gpio led for toggle led action
#define TOGGLE_GPIO CONFIG_BLINK_GPIO

void configure_led(void);
void blink_led(int s_led_state);