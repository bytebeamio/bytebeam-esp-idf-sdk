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

#endif /* BYTEBEAM_STREAM_H */