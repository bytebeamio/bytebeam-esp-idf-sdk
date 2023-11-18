#ifndef BYTEBEAM_CLIENT_H
#define BYTEBEAM_CLIENT_H

#include "mqtt_client.h"

/*This macro is used to specify the maximum length of bytebeam broker url string*/
#define BYTEBEAM_BROKER_URL_STR_LEN 100

/*This macro is used to specify the maximum length of bytebeam device id string*/
#define BYTEBEAM_DEVICE_ID_STR_LEN 10

/*This macro is used to specify the maximum length of bytebeam project id string*/
#define BYTEBEAM_PROJECT_ID_STR_LEN 100

/*This macro is used to specify the maximum number of actions that need to be handled for particular device*/
#define BYTEBEAM_NUMBER_OF_ACTIONS 10

struct bytebeam_client_t;
typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;

/**
 * @struct bytebeam_device_config_t
 * This sturct contains authentication configurations for particular device created on Bytebeam platform
 * @var bytebeam_device_config_t::ca_cert_pem
 * Certificate signed by CA
 * @var bytebeam_device_config_t::client_cert_pem
 * Client certificate
 * @var bytebeam_device_config_t::client_key_pem
 * Device private key
 * @var bytebeam_device_config_t::broker_uri
 * URL of the MQTT broker
 * @var bytebeam_device_config_t::device_id
 * Device identification issued by Bytebeam platform for particular device
 * @var bytebeam_device_config_t::project_id
 * Identification of tenant to which particular device belongs.
 */
typedef struct bytebeam_device_config {
    char *ca_cert_pem;
    char *client_cert_pem;
    char *client_key_pem;
    char broker_uri[BYTEBEAM_BROKER_URL_STR_LEN];
    char device_id[BYTEBEAM_DEVICE_ID_STR_LEN];
    char project_id[BYTEBEAM_PROJECT_ID_STR_LEN];
} bytebeam_device_config_t;

/**
 * @struct bytebeam_action_functions_map_t
 * This sturct contains name and function pointer for particular action 
 * @var bytebeam_action_functions_map_t::name
 * Name of particular action 
 * @var bytebeam_action_functions_map_t::func
 * Pointer to action handler function for particular action
 */
typedef struct bytebeam_action_functions_map {
    const char *name;
    int (*func)(struct bytebeam_client_t *bytebeam_client, char *args, char *action_id);
} bytebeam_action_functions_map_t;

typedef struct bytebeam_device_shadow_stream {
    uint64_t sequence;
    unsigned long long milliseconds;
    const char *status;
    const char *software_type;
    const char *software_version;
    const char *hardware_type;
    const char *hardware_version;
    const char* reboot_reason;
    long long uptime;
} bytebeam_device_shadow_stream_t;

typedef struct bytebeam_device_shadow {
    bytebeam_device_shadow_stream_t stream;
} bytebeam_device_shadow_t;

/**
 * @struct bytebeam_client_t
 * This struct contains all the configuration for instance of MQTT client
 * @var bytebeam_client_t::device_cfg
 * structure for all the tls authentication related configs
 * @var bytebeam_client_t::client
 * ESP MQTT client handle 
 * @var bytebeam_client_t::mqtt_cfg
 * ESP MQTT client configuration structure
 * @var bytebeam_client_t::action_funcs
 * Array containing action handler structure for all the configured actions on Bytebeam platform
 * @var bytebeam_client_t::connection_status
 * Connection status of MQTT client instance.
 */
typedef struct bytebeam_client {
    bytebeam_device_config_t device_cfg;
    bytebeam_client_handle_t client;
    bytebeam_client_config_t mqtt_cfg;
    bytebeam_action_functions_map_t action_funcs[BYTEBEAM_NUMBER_OF_ACTIONS];
    bytebeam_device_shadow_t device_shadow;
    int connection_status;
    bool use_device_config_data;
} bytebeam_client_t;

/*Status codes propogated via functions*/
typedef enum bytebeam_err {
    BB_SUCCESS = 0,
    BB_FAILURE = -1,
    BB_NULL_CHECK_FAILURE = -2,
    BB_PROGRESS_OUT_OF_RANGE = -3
} bytebeam_err_t;

/**
 * @brief Initializes bytebeam MQTT client.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      BB_SUCCESS: Bytebeam client successfully initialized
 *      BB_FAILURE: Bytebeam client initialization failed
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client is NULL
 */
bytebeam_err_t bytebeam_init(bytebeam_client_t *bytebeam_client);

/**
 * @brief Starts bytebeam MQTT client after client is initialised.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      - BB_SUCCESS: Bytebeam client started successfully
 *      - BB_FAILURE: Bytebeam client start failed
 *      - BB_NULL_CHECK_FAILURE: If the bytebeam_client is NULL
 */
bytebeam_err_t bytebeam_start(bytebeam_client_t *bytebeam_client);

/**
 * @brief Stops bytebeam MQTT client after client is started.
 *
 * @note  Cannot be called from the action function handler
 *
 * @param[in] bytebeam_client bytebeam client handle
 *
 * @return
 *      - BB_SUCCESS: Bytebeam client stopped successfully
 *      - BB_FAILURE: Bytebeam client stop failed
 *      - BB_NULL_CHECK_FAILURE: If the bytebeam_client is NULL
 */
bytebeam_err_t bytebeam_stop(bytebeam_client_t *bytebeam_client);

/**
 * @brief Destroys bytebeam MQTT client after the client is initialized
 *
 * @note  Cannot be called from the action function handler
 *
 * @param[in] bytebeam_client bytebeam client handle
 *
 * @return
 *      BB_SUCCESS: Bytebeam client successfully destroyed
 *      BB_FAILURE: Bytebeam client destroy failed
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client is NULL
 */
bytebeam_err_t bytebeam_destroy(bytebeam_client_t *bytebeam_client);

#endif /* BYTEBEAM_CLIENT_H */