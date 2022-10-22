/**
 * @file bytebeam_sdk.h
 * @brief This file contains definition of data types, function prototypes and macros which
 * can be used by applications to integrate SDK.
 */
#ifndef BYTEBEAM_SDK_H
#define BYTEBEAM_SDK_H

#include "mqtt_client.h"

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
    char broker_uri[100];
    char device_id[10];
    char project_id[100];
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
    int connection_status;
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
 *      BB_SUCCESS : Bytebeam Client successfully initialised
 *      BB_FAILURE : Bytebeam Client initialisation failed 
 */
bytebeam_err_t bytebeam_init(bytebeam_client_t *bytebeam_client);

/**
 * @brief Publish response message after particular action is completed.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] action_id       action id for particular action
 * 
 * @return
 *      BB_SUCCESS : Message publish successful
 *      BB_FAILURE : Message publish failed
 */
bytebeam_err_t bytebeam_publish_action_completed(bytebeam_client_t *bytebeam_client, char *action_id);

/**
 * @brief Publish response message after particular action execution has failed.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] action_id       action id for particular action
 * 
 * @return
 *      BB_SUCCESS : Message publish successful
 *      BB_FAILURE : Message publish failed
 */
bytebeam_err_t bytebeam_publish_action_failed(bytebeam_client_t *bytebeam_client, char *action_id);

/**
 * @brief Publish message indicating progress of action being executed.
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] action_id           action id for particular action
 * @param[in] progress_percentage action execution progress in percentage
 * 
 * @return
 *      BB_SUCCESS : Message publish successful
 *      BB_FAILURE : Message publish failed
 */
bytebeam_err_t bytebeam_publish_action_progress(bytebeam_client_t *bytebeam_client, char *action_id, int progress_percentage);

int bytebeam_publish_to_stream(bytebeam_client_t *bytebeam_client, char *stream_name, char *payload);

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
 * @brief Adds action handler for handling particular action.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] func_ptr        pointer to action handler function
 * @param[in] func_name       action name 
 * 
 * @return
 *      BB_SUCCESS : Action handler added successfully
 *      BB_FAILURE : Failure in adding action handler 
 */
bytebeam_err_t bytebeam_add_action_handler(bytebeam_client_t *bytebeam_client, int (*func_ptr)(bytebeam_client_t *, char *, char *), char *func_name);

/**
 * @brief Download and update Firmware image 
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] payload_string  buffer which stores payload received with OTA update init request
 * @param[in] action_id       action id for OTA update which is received with OTA update init request 
 * 
 * @return
 *      BB_SUCCESS : FW downloaded and updated successfully
 *      BB_FAILURE : FW download failed
 */
bytebeam_err_t handle_ota(bytebeam_client_t *bytebeam_client, char *payload_string, char *action_id);

#endif