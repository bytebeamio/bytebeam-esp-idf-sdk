#include "led_strip.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_driver.h"

static const char *TAG = "LED_DRIVER";

#if CONFIG_BLINK_LED_RMT

static led_strip_handle_t led_strip;

void blink_led(int s_led_state)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

oid configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = TOGGLE_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

void blink_led(int s_led_state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    ESP_LOGI(TAG,"Setting GPIO:%d to pin:%d\n", s_led_state, TOGGLE_GPIO);
    gpio_set_level(TOGGLE_GPIO, s_led_state);
}

void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(TOGGLE_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(TOGGLE_GPIO, GPIO_MODE_OUTPUT);
}

#endif