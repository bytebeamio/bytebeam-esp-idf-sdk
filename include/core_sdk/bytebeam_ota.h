#ifndef BYTEBEAM_OTA_H
#define BYTEBEAM_OTA_H

#include "bytebeam_client.h"

/*This macro is used to specify the maximum length of bytebeam OTA url string*/
#define BYTEBAM_OTA_URL_STR_LEN 200

/*This macro is used to specify the maximum length of the bytebeam OTA error string*/
#define BYTEBEAM_OTA_ERROR_STR_LEN 200

/**
 * @brief Download and update Firmware image 
 *
 * @param[in] bytebeam_client bytebeam client handle
 * @param[in] payload_string  buffer which stores payload received with OTA update init request
 * @param[in] action_id       action id for OTA update which is received with OTA update init request 
 * 
 * @return
 *       BB_SUCCESS: OTA firmware update handled successfully
 *       BB_NULL_CHECK_FAILURE: If the bytebeam_client, payload_string, or action_id is NULL
 *       BB_FAILURE: Firmware upgrade failed due to an error in parsing OTA JSON or performing OTA
 */
bytebeam_err_t handle_ota(bytebeam_client_t *bytebeam_client, char *payload_string, char *action_id);

#endif /* BYTEBEAM_OTA_H */