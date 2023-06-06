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
typedef struct bytebeam_device_config_t {
    char *ca_cert_pem;
    char *client_cert_pem;
    char *client_key_pem;
    char broker_uri[BYTEBEAM_BROKER_URL_STR_LEN];
    char project_id[BYTEBEAM_PROJECT_ID_STR_LEN];
    char device_id[BYTEBEAM_DEVICE_ID_STR_LEN];
} bytebeam_device_config_t;

typedef esp_mqtt_client_handle_t bytebeam_client_handle_t;
typedef esp_mqtt_client_config_t bytebeam_client_config_t;

struct bytebeam_client;

/**
 * @struct bytebeam_action_functions_map_t
 * This sturct contains name and function pointer for particular action 
 * @var bytebeam_action_functions_map_t::name
 * Name of particular action 
 * @var bytebeam_action_functions_map_t::func
 * Pointer to action handler function for particular action
 */
typedef struct {
    const char *name;
    int (*func)(struct bytebeam_client *bytebeam_client, char *args, char *action_id);
} bytebeam_action_functions_map_t;

typedef struct bytebeam_device_info {
    const char *status;
    const char *software_type;
    const char *software_version;
    const char *hardware_type;
    const char *hardware_version;
} bytebeam_device_info_t;

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
    bytebeam_device_info_t device_info;
    bytebeam_device_config_t device_cfg;
    bytebeam_client_handle_t client;
    bytebeam_client_config_t mqtt_cfg;
    bytebeam_action_functions_map_t action_funcs[BYTEBEAM_NUMBER_OF_ACTIONS];
    int connection_status;
    bool use_device_config_data;
} bytebeam_client_t;

/*Status codes propogated via functions*/
typedef enum {
    BB_SUCCESS = 0,
    BB_FAILURE = -1
} bytebeam_err_t;

/**
 * @brief Initializes bytebeam MQTT client.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      BB_SUCCESS : Bytebeam Client successfully initialized
 *      BB_FAILURE : Bytebeam Client initialization failed
 */
bytebeam_err_t bytebeam_init(bytebeam_client_t *bytebeam_client);

/**
 * @brief Starts bytebeam MQTT client after client is initialised.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      BB_SUCCESS : Bytebeam Client started successfully 
 *      BB_FAILURE : Bytebeam Client start failed 
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
 *      BB_SUCCESS : Bytebeam Client stopped successfully
 *      BB_FAILURE : Bytebeam Client stop failed
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
 *      BB_SUCCESS : Bytebeam Client successfully destroyed
 *      BB_FAILURE : Bytebeam Client destroy failed
 */
bytebeam_err_t bytebeam_destroy(bytebeam_client_t *bytebeam_client);

#endif /* BYTEBEAM_CLIENT_H */