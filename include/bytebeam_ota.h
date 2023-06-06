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
 *      BB_SUCCESS : FW downloaded and updated successfully
 *      BB_FAILURE : FW download failed
 */
bytebeam_err_t handle_ota(bytebeam_client_t *bytebeam_client, char *payload_string, char *action_id);

#endif /* BYTEBEAM_OTA_H */