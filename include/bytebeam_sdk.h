/**
 * @file bytebeam_sdk.h
 * @brief This file contains definition of data types, function prototypes and macros which
 * can be used by applications to integrate SDK.
 */
#ifndef BYTEBEAM_SDK_H
#define BYTEBEAM_SDK_H

#include "bytebeam_client.h"
#include "bytebeam_log.h"

/*This macro is used to specify the maximum length of bytebeam mqtt topic string*/
#define BYTEBEAM_MQTT_TOPIC_STR_LEN 200

/*This macro is used to specify the maximum length of bytebeam action id string*/
#define BYTEBEAM_ACTION_ID_STR_LEN 20

/*This macro is used to specify the maximum length of bytebeam OTA url string*/
#define BYTEBAM_OTA_URL_STR_LEN 200

/*This macro is used to specify the maximum length of the bytebeam OTA error string*/
#define BYTEBEAM_OTA_ERROR_STR_LEN 200

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

/**
 * @brief Publish message indicating action status of action being executed.
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] action_id           action id for particular action
 * @param[in] percentage          action execution progress in percentage
 * @param[in] status              action state of the execution
 * @param[in] error_message       error message if action failed
 *
 * @return
 *      BB_SUCCESS : Message publish successful
 *      BB_FAILURE : Message publish failed
 */
bytebeam_err_t bytebeam_publish_action_status(bytebeam_client_t* client, char *action_id, int percentage, char *status, char *error_message);

/**
 * @brief Publish message to particualar stream
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] stream_name         name of the target stream
 * @param[in] payload             message to publish
 * 
 * @return
 *      BB_SUCCESS : Message publish successful
 *      BB_FAILURE : Message publish failed
 */
bytebeam_err_t bytebeam_publish_to_stream(bytebeam_client_t *bytebeam_client, char *stream_name, char *payload);

/**
 * @brief Adds action handler for handling particular action.
 *
 * @note Arguments of the action handler function is generated dynamically inside the SDK and it will be freed after
 *       executing the action handler function. So make sure you are not using it outside the scope of the action
 *       handler function as this may cause memory leaks in your application.
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
 * @brief Remove action handler from the array
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] func_name       action name 
 * 
 * @return
 *      BB_SUCCESS : Action handler removed successfully
 *      BB_FAILURE : Failure in removing action handler 
 */

bytebeam_err_t bytebeam_remove_action_handler(bytebeam_client_t *bytebeam_client, char *func_name);

/**
 * @brief Update action handler for handling particular action.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] new_func_ptr    pointer to action handler function
 * @param[in] func_name       action name 
 * 
 * @return
 *      BB_SUCCESS : Action handler updated successfully
 *      BB_FAILURE : Failure in updating action handler 
 */
bytebeam_err_t bytebeam_update_action_handler(bytebeam_client_t *bytebeam_client, int (*new_func_ptr)(bytebeam_client_t *, char *, char *), char *func_name);

/**
 * @brief Check if action handler exists.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] func_name       action name
 *
 * @return
 *      BB_SUCCESS : If Action handler Exists
 *      BB_FAILURE : Action handler didn't exists
 */
bytebeam_err_t bytebeam_is_action_handler_there(bytebeam_client_t *bytebeam_client, char *func_name);

/**
 * @brief print action handler array.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      void
 */
void bytebeam_print_action_handler_array(bytebeam_client_t *bytebeam_client);

/**
 * @brief reset action handler array.
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      void
 */
void bytebeam_reset_action_handler_array(bytebeam_client_t *bytebeam_client);

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

#endif /* BYTEBEAM_SDK_H */