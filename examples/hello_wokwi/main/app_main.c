/*
 * @Brief
 * This example shows how to connect an ESP device to the Bytebeam cloud.
 * This is good starting point if you are looking to remotely update your ESP device
 * or push a command to it or stream data from it and visualise it on the Bytebeam Cloud.
 */

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

#include "esp_sntp.h"
#include "esp_log.h"

#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cJSON.h"
#include "driver/gpio.h"
#include "bytebeam_sdk.h"

// this macro is used to specify the delay for 1 sec.
#define APP_DELAY_ONE_SEC 1000u

// this macro is used to specify the maximum length of led status string
#define LED_STATUS_STR_LEN 200

// this macro is used to specify the gpio led for toggle led action
#define TOGGLE_GPIO 2

static int config_toggle_period = APP_DELAY_ONE_SEC;

static uint32_t led_state = 0;
static int toggle_led_cmd = 0;

static char led_status[LED_STATUS_STR_LEN] = "";
static char device_shadow_stream[] = "device_shadow";

static bytebeam_client_t bytebeam_client;

// put your device config data here
static const char* project_id = "demo";
static const char* device_id  = "1";
static const char* broker_uri = "mqtts://cloud.bytebeam.io:8883";
static const char* ca_cert     = "-----BEGIN CERTIFICATE-----\nMIIFrDCCA5SgAwIBEOMAwGA1UEBhMFSW5k\naWExETAPBgNVBAgTCEthcm5hdGFrMRIwEAYDVQQWxvcmUxFzAVBgNV\nBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIxMDkwMjExMDYyM1oXDTMxMDkwMjExMDYyM1owdzEOMAwG\nA1UEBhMFSW5kaWExETAPBgNVBEthcm5hdGFrMRIwEAYDVQQHEwlCYW5nYWxv\ncmUxFzAVBgNVBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDAS\nBgNVBAoTC0J5dGViZWFtLmlvMIINBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKC\nAgEAr/bnOa/8AUGZmd/s+7rejuROgeLqqU9X15KKfKOBqcoMyXsSO65UEwdpw\nMl7GDCdHqFTymqdnAnbhgaT1PoIFhOG64y7UiNgiWmbh0XJj8G6oLrW9rQ1gug1Q\n/D7x2fUnza71aixiwEL+KsIFYIdDuzmoRD3rSer/bKOcGGs0WfB54KqIVVZ1DwsU\nk1wx5ExsKo7gAdXMAbdHRI2Szmn5MsZwGL6V0LfsKLE8ms2qlZe50oo2woLNN6XP\nRfRL4bwwkdsCqXWkkt4eUSNDq9hJsuINHdhO3GUieLsKLJGWJ0lq6si74t75rIKb\nvvsFEQ9mnAVS+iuUUsSjHPJIMnn/Nmgl/R/8FP5TUgUrHvHXKQkJ9h/a7+3tS\nlV2KMsFksXaFrGEByGIJ7yR4qu9hx5MXf8pf8EGEwOW/H3CdWcC2MvJ11PVpceUJ\neDVwE7B4gPM9Kx02RNwvUMH2FmYqkXX2DrrHQGQuq+6VRoN3rEdmGPqnONJEPeOw\nZzcGDVXKWZtd7UCbcZKdn0RYmVtI/OB5OW8IRoXFYgGB3IWP796dsXIwbJSqRb9m\nylICGOceQy3VR+8+BHkQLj5/ZKTe+AA3Ktk9UADvxRiWKGcejSA/LvyT8qzz0dqn\nGtcHYJuhJ/XpkHtB0PykB5WtxFjx3G/osbZfrNflcQZ9h1MCAwEAAaNCMEAwDgYD\nVR0PAQH/BAQDAgKEMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFKl/MTbLrZ0g\nurneOmAfBHO+LHz+MA0GCSqGSIb3DQEBCwUAA4ICAQAlus/uKic5sgo1d2hBJ0Ak\ns1XJsA2jz+OEdshQHmCCmzFir3IRSuVRmDBaBGlJDHCELqYxKn6dl/sKGwoqoAQ5\nOeR2sey3Nmdyw2k2JTDx58HnApZKAVir7BDxbIbbHmfhJk4ljeUBbertNXWbRHVr\ncs4XBNwXvX+noZjQzmXXK89YBsV2DCrGRAUeZ4hQEqV7XC0VKmlzEmfkr1nibDr5\nqwbI+7QWIAnkHggYi27lL2UTHpbsy9AnlrRMe73upiuLO7TvkwYC4TyDaoQ2ZRpG\nHY+mxXLdftoMv/ZvmyjOPYeTRQbfPqoRqcM6XOPXwSw9B6YddwmnkI7ohNOvAVfD\nwGptUc5OodgFQc3waRljX1q2lawZCTh58IUf32CRtOEL2RIz4VpUrNF/0E2vts1f\npO7V1vY2Qin998Nwqkxdsll0GLtEEE9hUyvk1F8U+fgjJ3Rjn4BxnCN4oCrdJOMa\nJCaysaHV7EEIMqrYP4jH6RzQzOXLd0m9NaL8A/Y9z2a96fwpZZU/fEEOH71t3Eo3\nV/CKlysiALMtsHfZDwHNpa6g0NQNGN5IRl/w1TS1izzjzgWhR6r8wX8OPLRzhNRz\n2HDbTXGYsem0ihC0B8uzujOhTHcBwsfxZUMpGjg8iycJlfpPDWBdw8qrGu8LeNux\na0cIevjvYAtVysoXInV0kg==\n-----END CERTIFICATE-----\n";
static const char* client_cert = "-----BEGIN CERTIFICATE-----\nMIIEaDCCAlCgAwIBAgICB+MwDQYJKoZIhvcAwGA1UEBhMFSW5k\naWExETAPBgNVBAgTCEthcRIwEAYDVQQHEwlCYW5nYWxvcmUxFzAVBgNV\nBAkTDlN1YmJpYWggR2FyZGVuMQ8wDQYDVQQREwY1NjAwMTExFDASBgNVBAoTC0J5\ndGViZWFtLmlvMB4XDTIzMDEwNzExMDYxNFoXDTMzMDEwNzExMDYxNFowHzERMA8G\nA1UEChMIcmVjdDExCjAIBgNVBAMTATEwggEiMA0GCSqGSIb3DQEBAQUAA4IB\nDwAwggEKAoIBAQDqzmT+vFykAZSxArOOS6QbtBKKIbheMnUOR4EOewxcDeo1ajdk\nlZB+Da9taRoSuxPBUThaorKn20qdSk8XxKDjcJ7ZTS8daflvEqF9bbVAf1kC\n1kq0u4SO9CNlPHUmMPio7TwFn2rq57qNgSQoja4VQ1Q+sl22xtKiWJ3RrRMK+Cp\n/i85wzQ6lYqJH2b7em023TRQKuZ/9Im/edPtMB3DJTaFT6f5AT82+y+BidUoQbFp\n/+k+t4tPntqsv2pGPGm+mQdw8Yl2XGrvCVLDd+rDETmwSJSH7i6F8XrwRmpbo4ln\nbfTKNsLO5W+FEIsjhktCe94o0v8GfCMbkjsJAgMBAAGjVjBUMA4GA1UdDwEB/wQE\nAwIFoDATBgNVHSUKBggrBgEFBQcDAjAfBgNVHSMEGDAWgBSpfzE2y62dILq5\n3jpgHwRzvix8/jAMBgNVHREEBTADggExMA0GCSqGSIb3DQEBCwUAA4ICAQBLCl5d\nBmaMbzybw8YhuQhGlZFtD3ox8VVV6PFl9wKhlgSAgCxvmy1ye1LsYhljMyFs2rSH\nfWj7yhmqvjQcYUQHNJTJlcy0jIDDh60jFi6b5RyJaqje1MKH1PJd09oUu14t6u2I\nvIH/D6h82Ohl1VwASSc0s3UsZrUz6GPWfP/iBN9qOtCKdyUQannP7uS9o6R6T5ks\nvEs0vp/N6VMYxXB4/39w/IIxgiSM4bIK52ZmVr0hkKRi20RPB2SPEqqKgERD6ORf\ndvtsk1ozB4ICrw8aVKqN/71TTaKwf0mzspw74xSCamhjM9PyVoJ84gNKt7l+ONqH\n02JmjKdA7iNk8WMd9UXFs8mlTtugm0gzfxzJ0dnSHlfllVDZtNkGHjXJb6RT+Fck\n6UpGhyCvZR8rpCTb4o3H+gkiPPE6+fA6SMQnSbA0L5BKjRt8QkHBedd4NSnTGQkO\nty7a1qq+54ui3fitp5slBGWJRcuRgpTNOaaIssIh2hrd48gQUgumCQEJXdGrsJiW\nYU17tHMd2bvsRvhB5sE70mEbHDV/WjQ17gv/oriHL2ffeblOCSdzo3MVXwqn55JJ\nnxdBltLjwhTPdr9bJYWqOy2gscYCyWm8YW3aXFYRnEdqc9vt2hy82kRvEP/rgDLQ\nApWvZdxAPZ2DtP5Bv/Xsmwd4IK04uNoLL3xaSw==\n-----END CERTIFICATE-----\n";
static const char* client_key  = "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAcpAGUsQKzjkukG7QSiiG4XjJ1DkeBDnsMXA3qNWo3\nZJWQfg2vbWkaErsTwfbb4UpPF8Sg43Ce2U0vHWn5bxKhfW21QH9Z\nAtZKtLuEjvQjZWjx1JjD4qO08BZ9q6ue6jYEkKI2uFUNUPrJdtsbSolid0a0TCvg\nqf4vOcM0OpWKiR9m+3ptNt00UCrmf/SJv3nT7TAdwyU2hU+n+QE/NvsvgYnVKEGx\naf/pPreLT59qRjxpvpkHcPGJdlxq7wlSw3fqwxE5sEiUh+4uhfF68EZqW6OJ\nZ230yjbCzuVvhRCLI4ZLQnveKNL/BnwjG5I7CQIDAQABAoIBAQC4vmOgObv6HnRL\nQYDc+I7KbUg9Fl4bOg9EwXPcCea42DV1ImhphYL9ZrBpBGBrQDuKAyF/znsa\nruNqFklcvgyIZtfUyXHEhtkG3YQJf1yHcB/BvQId3G4uEYRSBzd1MRq0btPG6MN\nKx8B0DaPRdIf/mtXx7ozlflHEAk1Jh5sPGo7FfzKNdFTONgueVsk4FdAvhWqjwe1\nZWu813yUnerzxKrHR/tpQRKh/mF/ol9Cr1XKARsOHaHzGjfagRlM9txvxkso8NEM\n1ckd0JLGpOg0NTwGzCJdxW5V5EJgH2U+gV8S7zdX2mFrbMyzPZt2ER2wJwGFKiVd\nRZ+ZX+rBAob3CFSV94XDfYZ0m3p/xeAhzYRtNvk8BLmSFdIyDP8nZweX4Q8P\nkGJRfPj/aV1RXjbjxTlgoSxrTdz8pG69q6b/SPRbBBQ/WR3eeHYi1SBGsm+Cs\n/Tt2hMgQrmGMaDmvU2t9WXdLwlJwLbRgTXAeXT7agg0AdtB+jBCECG4NAoGBAPNl\nfQni5OwlnNJUCTfHA32Ggl8AeQ9TmDajVZD+yHj2+7V30ugSJDiPrwfnnOy1pGKb\nPIp/p5gmhL+Jv3xfl6+WOgKNXv+1ZgJXf+avxznsjDJ8kLZFC8s/Kt1BwVw2j6ld\nlGI1E06m0bq2YiOPoX2zQn0ja3Xx/QF5Z/nQKH3tAoGBAPMxYCqrYwt97mYw/cor\neJe1scPuKGxWBfEXQhWy48l+6542mZFTKkMnK4GnQgJ61eQHd3eBCDXNlOdAgkvq\nytWwHg9CcUne98qu2qPEuCCfI1EEQJuGvfeIRkkc5Bt7QEdNtz/qN+eZ2smDXf55\ntsfpnrj+Q4K6KkY114eL/SEhAoGABJFtDVhyz3PNQ61MFv3nNN3naH0LyJvbCXt7\nSaNeg9au1rMPBEgC4gTgVhekvFSUZz1en4LR1Gs0ppuhOmZY5HS/VfjtRYNUDMsz\nnVLDPHUSFIh99s3I8OGYnlpzlJV5kMYu2MECQcGjwgtWQDCb6U4J+2tEwwvtoa9z\noh6kA+kCgYBzCYVrWjClt135EVf6mw6g6iQPM4rJB1DbKag7GA0+D6UzpaX0mowD\nptH3njEz5eVndpMvAbrO23hxrc+PMRBqzHB29lfyOPO8SfUFKynj3LSqHi9Zp6oC\nvZSPTqsikPZKSOdmjOgJ0aP6JLZhgop8qaUuQG9pgTvARBw87h3nuQ==\n-----END RSA PRIVATE KEY-----\n";

static const char *TAG = "BYTEBEAM_HELLO_WOKWI_EXAMPLE";

static void set_led_status(void)
{
    int max_len = LED_STATUS_STR_LEN;
    int temp_var = snprintf(led_status, max_len, "LED is %s !", led_state == true ? "ON" : "OFF");

    if(temp_var >= max_len)
    {
        ESP_LOGE(TAG, "led status string size exceeded max length of buffer");
    }
}

static void toggle_led(void)
{
    // toggle the gpio led state
    led_state = !led_state;
    ESP_LOGI(TAG, " LED_%s!", led_state == true ? "ON" : "OFF");

    // set the gpio led level according to the state (low or high)
    gpio_set_level(TOGGLE_GPIO, led_state);
}

static void configure_led(void)
{
    // set the gpio led pin as output
    gpio_reset_pin(TOGGLE_GPIO);
    gpio_set_direction(TOGGLE_GPIO, GPIO_MODE_OUTPUT);
}

static int publish_device_shadow(bytebeam_client_t *bytebeam_client)
{   
    struct timeval te;
    long long milliseconds = 0;
    static uint64_t sequence = 0;

    cJSON *device_shadow_json_list = NULL;
    cJSON *device_shadow_json = NULL;
    cJSON *sequence_json = NULL;
    cJSON *timestamp_json = NULL;
    cJSON *device_status_json = NULL;

    char *string_json = NULL;

    device_shadow_json_list = cJSON_CreateArray();

    if(device_shadow_json_list == NULL)
    {
        ESP_LOGE(TAG, "Json Init failed.");
        return -1;
    }

    device_shadow_json = cJSON_CreateObject();

    if(device_shadow_json == NULL)
    {
        ESP_LOGE(TAG, "Json add failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    // get the current time
    gettimeofday(&te, NULL); 
    milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    // make sure you got the epoch millis
    if(milliseconds == 0)
    {
        ESP_LOGE(TAG, "failed to get epoch millis.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    timestamp_json = cJSON_CreateNumber(milliseconds);

    if(timestamp_json == NULL)
    {
        ESP_LOGE(TAG, "Json add time stamp failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "timestamp", timestamp_json);

    sequence++;
    sequence_json = cJSON_CreateNumber(sequence);

    if(sequence_json == NULL)
    {
        ESP_LOGE(TAG, "Json add sequence id failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "sequence", sequence_json);

    device_status_json = cJSON_CreateString(led_status);

    if(device_status_json == NULL)
    {
        ESP_LOGE(TAG, "Json add device status failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    cJSON_AddItemToObject(device_shadow_json, "Status", device_status_json);

    cJSON_AddItemToArray(device_shadow_json_list, device_shadow_json);

    string_json = cJSON_Print(device_shadow_json_list);

    if(string_json == NULL)
    {
        ESP_LOGE(TAG, "Json string print failed.");
        cJSON_Delete(device_shadow_json_list);
        return -1;
    }

    ESP_LOGI(TAG, "\nStatus to send:\n%s\n", string_json);

    // publish the json to device shadow stream
    int ret_val = bytebeam_publish_to_stream(bytebeam_client, device_shadow_stream, string_json);

    cJSON_Delete(device_shadow_json_list);
    cJSON_free(string_json);

    return ret_val;
}

static void app_start(bytebeam_client_t *bytebeam_client)
{
    int ret_val = 0;

    while (1) 
    {
        if (toggle_led_cmd == 1)
        {
            // toggle the led
            toggle_led();
            
            // set led status
            set_led_status();

            // pulish led status to device shadow
            ret_val = publish_device_shadow(bytebeam_client);

            if (ret_val != 0)
            {
                ESP_LOGE(TAG, "Publish to device shadow failed");
            }

            // reset the toggle led command flag
            toggle_led_cmd = 0;
        }

        vTaskDelay(config_toggle_period / portTICK_PERIOD_MS);
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
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) 
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    time(&now);
    localtime_r(&now, &timeinfo);
}

// handler for toggle led action
int handle_toggle_led(bytebeam_client_t *bytebeam_client, char *args, char *action_id)
{
    int ret_val = 0;

    // set the toggle led command flag
    toggle_led_cmd = 1;

    // publish action completed response
    ret_val = bytebeam_publish_action_completed(bytebeam_client, action_id);

    if(ret_val != 0) 
    {
        ESP_LOGE(TAG, "Failed to Publish action completed response for Toggle LED action");
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

    // sync time from the ntp
    sync_time_from_ntp();

    // configure the gpio led
    configure_led();

    // setting up the device config data
    bytebeam_client.device_cfg.ca_cert_pem     = ca_cert;
    bytebeam_client.device_cfg.client_cert_pem = client_cert;
    bytebeam_client.device_cfg.client_key_pem  = client_key;
    strcpy(bytebeam_client.device_cfg.project_id, project_id);
    strcpy(bytebeam_client.device_cfg.broker_uri, broker_uri);
    strcpy(bytebeam_client.device_cfg.device_id, device_id);

    // set the flag to use the provided device config data
    bytebeam_client.use_device_config_data = true;

    // initialize the bytebeam client
    bytebeam_init(&bytebeam_client);

    // add the handler for toggle led action
    bytebeam_add_action_handler(&bytebeam_client, handle_toggle_led, "toggle_led");

    // start the bytebeam client
    bytebeam_start(&bytebeam_client);

    //
    // start the main application
    //
    app_start(&bytebeam_client);
}