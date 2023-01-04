/**
 * Brief
 * This example shows how to connect an ESP device to the Bytebeam cloud.
 * This is good starting point if you are looking to remotely update your ESP device
   or push a command to it or stream data from it and visualise it on the Bytebeam Cloud.
*/

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_tls.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "cJSON.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "bytebeam_sdk.h"

/*This macro is used to test the action handling feature*/
#define BYTEBEAM_ACTION_HANDLING_TEST 0

/*This macro is used to test the bytebeam log feature*/
#define BYTEBEAM_LOG_TEST 0

/*This macro is used to specify the GPIO LED for toggle_board_led action*/
#define BLINK_GPIO 2

/*his macro is used to specify the GPIO LED for update_config action*/
#define UPDATE_GPIO 13

#define LEDC_TIMER              LEDC_TIMER_0            // Ledc Timer 0 Peripheral  
#define LEDC_MODE               LEDC_LOW_SPEED_MODE     // Ledc Low Speed Mode 
#define LEDC_OUTPUT_IO          UPDATE_GPIO             // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0          // Ledc Channel0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT       // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000)                  // Frequency in Hertz. Set frequency at 5 kHz
#define LEDC_STEP_SIZE          255                     // Set the Step size with 8 bits resolution

static uint8_t led_state = 0;
static int config_blink_period = 1000;
static int toggle_led_cmd = 0;

static uint32_t led_duty_cycle = 0;
static int update_config_cmd = 0;
static char *update_config_str = NULL;

char led_status[200];

bytebeam_client_t bytebeam_client;

static const char *TAG = "BYTEBEAM_DEMO_EXAMPLE";

#if BYTEBEAM_ACTION_HANDLING_TEST
    int hello_world(bytebeam_client_t *bytebeam_client, char *args, char *action_id) 
    {
        ESP_LOGI(TAG, "Hello World !");
        return 0;
    }

    void action_handling_positive_test() 
    {
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_1");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_2");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_3");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_4");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_5");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_6");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_7");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_8");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_9");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_10");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_remove_action_handler(&bytebeam_client, "hello_2");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_update_action_handler(&bytebeam_client, hello_world, "hello_4");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_reset_action_handler_array(&bytebeam_client);
        bytebeam_print_action_handler_array(&bytebeam_client);

        ESP_LOGI(TAG, "Action Handling Positive Test Executed Successfully !\n");
    }

    void action_handling_negative_test() 
    {
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_1");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_1");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_2");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_3");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_4");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_5");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_6");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_7");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_8");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_9");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_10");
        bytebeam_add_action_handler(&bytebeam_client, hello_world, "hello_11");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_remove_action_handler(&bytebeam_client, "hello_22");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_update_action_handler(&bytebeam_client, hello_world, "hello_44");
        bytebeam_print_action_handler_array(&bytebeam_client);

        bytebeam_reset_action_handler_array(&bytebeam_client);
        bytebeam_print_action_handler_array(&bytebeam_client);

        ESP_LOGI(TAG, "Action Handling Negative Test Executed Successfully !\n");
    }
#endif

#if BYTEBEAM_LOG_TEST
    void bytebeam_log_test() 
    {
        /* default bytebeam log level is BYTEBEAM_LOG_LEVEL_INFO, So Logs beyond Info level will not work :) 
         * You can always chnage the log setting at the compile time or run time.
         */
        BYTEBEAM_LOGE(TAG, "I am %s Log", "Error");                
        BYTEBEAM_LOGW(TAG, "I am %s Log", "Warn");                 
        BYTEBEAM_LOGI(TAG, "I am %s Log", "Info");                 
        BYTEBEAM_LOGD(TAG, "I am %s Log", "Debug");                   // Debug Log will not appear to cloud
        BYTEBEAM_LOGV(TAG, "I am %s Log", "Verbose");                 // Verbose Log will not appear to cloud

        /* Changing log level to Verbose for showing use case */   
        esp_log_level_set(TAG, ESP_LOG_VERBOSE);        
        bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL_VERBOSE);       

        /* Now bytebeam log level is BYTEBEAM_LOG_LEVEL_VERBOSE, So every Logs should work now :)
         * Make sure your esp log level supports Verbose to see the log in the terminal.
         */
        BYTEBEAM_LOGE(TAG, "This is %s Log", "Error");                
        BYTEBEAM_LOGW(TAG, "This is %s Log", "Warn");                 
        BYTEBEAM_LOGI(TAG, "This is %s Log", "Info");                 
        BYTEBEAM_LOGD(TAG, "This is %s Log", "Debug");                // Debug Log should appear to cloud
        BYTEBEAM_LOGV(TAG, "This is %s Log", "Verbose");              // Verbose Log should appear to cloud

        /* Changing log level back to Info for meeting initail conditions */
        esp_log_level_set(TAG, ESP_LOG_INFO);
        bytebeam_log_level_set(BYTEBEAM_LOG_LEVEL_INFO);

        ESP_LOGI(TAG, "Bytebeam Log Test Executed Successfully !\n");
    }
#endif

static void set_toggle_led_status()
{
    int max_len = 200;
    int temp_var = snprintf(led_status, max_len, "LED is %s !", led_state == true ? "ON" : "OFF");

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "toggle LED device status string size exceeded max length of buffer");
    }
}

static void toggle_led(void)
{
    /* Toggle the led state */
    led_state = !led_state;
    ESP_LOGI(TAG, " LED_%s!", led_state == true ? "ON" : "OFF");

    /* Set the GPIO level according to the state (LOW or HIGH) */
    gpio_set_level(BLINK_GPIO, led_state);
}

static void configure_led(void)
{
    /* Set the Blink GPIO pin as OUTPUT */
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

static void set_update_led_status() 
{
    int max_len = 200;
    uint32_t ledc_res = (uint32_t)round(pow(2,LEDC_DUTY_RES)) - 1;
    float brightness = (led_duty_cycle/(float)ledc_res) * 100;

    int temp_var = snprintf(led_status, max_len, "LED Brighntness is %.2f%% !", brightness);

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "update LED device status string size exceeded max length of buffer");
    }
}

static void update_led(void) 
{
    /* Set the new duty cycle for Update GPIO pin */
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, led_duty_cycle);
    ESP_LOGI(TAG, " LED Duty Cycle Value is %d!", (int)led_duty_cycle);

    /* Update the new duty cycle for Update GPIO pin */
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static int publish_device_shadow(bytebeam_client_t *bytebeam_client)
{
    static uint64_t sequence = 0;

    cJSON *device_shadow_json_list = NULL;
    cJSON *device_shadow_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *device_status_json = NULL;

    char *string_json = NULL;

    device_shadow_json_list = cJSON_CreateArray();

    if (device_shadow_json_list == NULL) {
        ESP_LOGE(TAG, "Json Init failed.");
        return -1;
    }

    device_shadow_json = cJSON_CreateObject();

    if (device_shadow_json == NULL) {
        ESP_LOGE(TAG, "Json add failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    timestamp_json = cJSON_CreateNumber(milliseconds);

    if (timestamp_json == NULL) {
        ESP_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if (sequence_json == NULL) {
        ESP_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "sequence", sequence_json);

    device_status_json = cJSON_CreateString(led_status);

    if (device_status_json == NULL) {
        ESP_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    cJSON_AddItemToArray(device_shadow_json_list, device_shadow_json);

    string_json = cJSON_Print(device_shadow_json_list);
    ESP_LOGI(TAG, "\nStatus to send:\n%s\n", string_json);

    int ret_val = bytebeam_publish_to_stream(bytebeam_client, "device_shadow", string_json);

    cJSON_Delete(device_shadow_json_list);
    free(string_json);

    return ret_val;
}

static void app_start(bytebeam_client_t *bytebeam_client)
{
    while (1) {
        if(update_config_cmd) {
            cJSON *root = NULL;
            cJSON *name = NULL;
            cJSON *version = NULL;
            cJSON *step_value = NULL;

            /* Parse the received json */
            root = cJSON_Parse(update_config_str);

            name = cJSON_GetObjectItem(root, "name");

            if (!(cJSON_IsString(name) && (name->valuestring != NULL))) {
                ESP_LOGE(TAG, "Error parsing update config name\n");
            }
            ESP_LOGI(TAG, "Checking update config name \"%s\"\n", name->valuestring);

            version = cJSON_GetObjectItem(root, "version");

            if (!(cJSON_IsString(version) && (version->valuestring != NULL))) {
                ESP_LOGE(TAG, "Error parsing update config version\n");
            }
            ESP_LOGI(TAG, "Checking update config version \"%s\"\n", version->valuestring);

            step_value = cJSON_GetObjectItem(root, "step_value");

            if (!(cJSON_IsNumber(step_value))) {
                ESP_LOGE(TAG, "Error parsing update config step value\n");
            }
            ESP_LOGI(TAG, "Checking update config step value %d\n", (int)step_value->valuedouble);
            
            /* Generate the duty cycle */
            uint32_t ledc_res = (uint32_t)round(pow(2,LEDC_DUTY_RES)) - 1;
            led_duty_cycle = ((ledc_res) * (step_value->valuedouble/LEDC_STEP_SIZE));
            
            /* Update the LED */
            update_led();

            /* Set update LED status */
            set_update_led_status();

            /* Pulish status to device shadow */
            int ret_val = publish_device_shadow(bytebeam_client);
            if (ret_val != 0) {
                ESP_LOGE(TAG, "Publish to device shadow failed");
            }

            update_config_str = NULL;
            update_config_cmd = 0;
        }

        if (toggle_led_cmd == 1) {
            /* Toggle the LED */
            toggle_led();
            
            /* Set toggle LED status */
            set_toggle_led_status();

            /* Pulish status to device shadow */
            int ret_val = publish_device_shadow(bytebeam_client);
            if (ret_val != 0) {
                ESP_LOGE(TAG, "Publish to device shadow failed");
            }

            toggle_led_cmd = 0;
        }

        vTaskDelay(config_blink_period / portTICK_PERIOD_MS);
    }
}

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif

    sntp_init();
}

static void sync_time_from_ntp(void)
{
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    initialize_sntp();

    // wait for time to be set
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    time(&now);
    localtime_r(&now, &timeinfo);
}

/* Handler for update_config action */
int handle_update_config(bytebeam_client_t *bytebeam_client, char *args, char *action_id) 
{
    update_config_cmd = 1;
    update_config_str = args;

    if ((bytebeam_publish_action_completed(bytebeam_client, action_id)) != 0) {
        ESP_LOGE(TAG, "Failed to Publish action response for Update Config action");
		return -1;
    }

    return 0;
}

/* Handler for toggle_board_led action */
int handle_toggle_led(bytebeam_client_t *bytebeam_client, char *args, char *action_id)
{
    toggle_led_cmd = 1;

    if ((bytebeam_publish_action_completed(bytebeam_client, action_id)) != 0) {
        ESP_LOGE(TAG, "Failed to Publish action response for Toggle LED action");
		return -1;
    }

    return 0;
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", (int)esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    sync_time_from_ntp();
    configure_led();
    ledc_init();

    bytebeam_init(&bytebeam_client);

    /* Action Handling can be tested once bytebeam client is initialized successfully and before adding
     * any action, enable BYTEBEAM_ACTION_HANDLING_TEST macro to test action handing feature
     */
#if BYTEBEAM_ACTION_HANDLING_TEST
    action_handling_positive_test();
    action_handling_negative_test();
#endif

    bytebeam_add_action_handler(&bytebeam_client, handle_ota, "update_firmware");
    bytebeam_add_action_handler(&bytebeam_client, handle_update_config, "update_config");
    bytebeam_add_action_handler(&bytebeam_client, handle_toggle_led, "toggle_board_led");
    bytebeam_start(&bytebeam_client);

    /* Bytebeam Logs can be tested once bytebeam client is started successfully, enable BYTEBEAM_LOG_TEST
     * macro to test bytebeam log feature
     */
#if BYTEBEAM_LOG_TEST
    bytebeam_log_test();
#endif

    /* Use the bytebeam_stop api to stop the bytebeam client at any point of time in the code, Also you
     * can mantain the bytebeam client start and bytebeam client stop flow as per you application needs
     */
    // bytebeam_stop(&bytebeam_client);

    /* Use the bytebeam_destroy api to destroy the bytebeam client at any point of time in the code, Also
     * you can mantain the bytebeam cient init and bytebeam client destroy flow as per you application needs
     */
    // bytebeam_destroy(&bytebeam_client);

    app_start(&bytebeam_client);
}
