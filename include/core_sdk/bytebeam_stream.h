#ifndef BYTEBEAM_STREAM_H
#define BYTEBEAM_STREAM_H

#include "bytebeam_client.h"

/**
 * @brief Publish message to particualar stream
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] stream_name         name of the target stream
 * @param[in] payload             message to publish
 * 
 * @return
 *      BB_SUCCESS: Message publish successful
 *      BB_FAILURE: Message publish failed
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client, stream_name, or payload is NULL
 */
bytebeam_err_t bytebeam_publish_to_stream(bytebeam_client_t *bytebeam_client, char *stream_name, char *payload);

/**
 * @brief Publish device shadow message
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * 
 * @return
 *      BB_SUCCESS: Message publish successful
 *      BB_FAILURE: Message publish failed
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client, stream_name, or payload is NULL
 */
bytebeam_err_t bytebeam_publish_device_shadow(bytebeam_client_t *bytebeam_client);

/**
 * @brief Add custom device shadow message
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] custom_json_str     custom device shaodw message
 * 
 * @return
 *      BB_SUCCESS: Message added successful
 *      BB_FAILURE: If Message size exceeded configured buffer size
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client or custom_json_str is NULL
 */
bytebeam_err_t bytebeam_add_custom_device_shadow(bytebeam_client_t *bytebeam_client, char *custom_json_str);

/**
 * @brief Register device shadow update handler
 *
 * @param[in] bytebeam_client     bytebeam client handle
 * @param[in] func_ptr            device shadow update handler
 * 
 * @return
 *      BB_SUCCESS: Handler registered successful
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client or func_ptr is NULL
 */
bytebeam_err_t bytebeam_register_device_shadow_updater(bytebeam_client_t *bytebeam_client, int (*func_ptr)(bytebeam_client_t *));

void bytebeam_user_thread_entry(void *pv);
void bytebeam_mqtt_thread_entry(void *pv);

#endif /* BYTEBEAM_STREAM_H */