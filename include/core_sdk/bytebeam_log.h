#ifndef BYTEBEAM_LOG_H
#define BYTEBEAM_LOG_H

#include "bytebeam_hal.h"
#include "bytebeam_client.h"

/*This macro is used to specify the maximum length of bytebeam log stream string*/
#define BYTEBEAM_LOG_STREAM_STR_LEN 20

#define BB_LOGX(BB_HAL_LOGX, level, tag, fmt, ...)                                            \
     do {                                                                                     \
        const char* levelStr = bytebeam_log_level_str[level];                                 \
        if(level <= bytebeam_log_level_get()) {                                               \
            if (bytebeam_log_publish(levelStr, tag, fmt, ##__VA_ARGS__) == BB_FAILURE) {      \
                BB_HAL_LOGE(tag, "Failed To Publish Bytebeam Log !");                         \
            } else {                                                                          \
                BB_HAL_LOGX(tag, fmt, ##__VA_ARGS__);                                         \
            }                                                                                 \
        }                                                                                     \
    } while (0)

#define BB_LOGE(tag, fmt, ...)  BB_LOGX(BB_HAL_LOGE, BB_LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)
#define BB_LOGW(tag, fmt, ...)  BB_LOGX(BB_HAL_LOGW, BB_LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)
#define BB_LOGI(tag, fmt, ...)  BB_LOGX(BB_HAL_LOGI, BB_LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)
#define BB_LOGD(tag, fmt, ...)  BB_LOGX(BB_HAL_LOGD, BB_LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)
#define BB_LOGV(tag, fmt, ...)  BB_LOGX(BB_HAL_LOGV, BB_LOG_LEVEL_VERBOSE, tag, fmt, ##__VA_ARGS__)

/* This enum represents Bytebeam Log Levels */
typedef enum {
    BB_LOG_LEVEL_NONE,
    BB_LOG_LEVEL_ERROR,
    BB_LOG_LEVEL_WARN,
    BB_LOG_LEVEL_INFO,
    BB_LOG_LEVEL_DEBUG,
    BB_LOG_LEVEL_VERBOSE,
} bytebeam_log_level_t;

static const char* bytebeam_log_level_str[6] = {
    [BB_LOG_LEVEL_NONE]    = "None",
    [BB_LOG_LEVEL_ERROR]   = "Error",
    [BB_LOG_LEVEL_WARN]    = "Warn",
    [BB_LOG_LEVEL_INFO]    = "Info",
    [BB_LOG_LEVEL_DEBUG]   = "Debug",
    [BB_LOG_LEVEL_VERBOSE] = "Verbose"
};

/**
 * @brief Set the bytebeam log client handle
 *
 * @param[in] bytebeam_client bytebeam client handle
 * 
 * @return
 *      void
 */
void bytebeam_log_client_set(bytebeam_client_t *bytebeam_client);

/**
 * @brief Enable the cloud logging
 *
 * @param
 *      void
 *
 * @return
 *      void
 */
void bytebeam_enable_cloud_logging();

/**
 * @brief Return the cloud logging status i.e Enabled or Disabled
 *
 * @param
 *      void
 *
 * @return
 *      True  : If cloud logging is enabled
 *      False : Cloud logging is disabled
 */
bool bytebeam_is_cloud_logging_enabled();

/**
 * @brief Disable the cloud logging
 *
 * @param
 *      void
 *
 * @return
 *      void
 */
void bytebeam_disable_cloud_logging();

/**
 * @brief Set the bytebeam log level
 *
 * @param[in] level log level
 * 
 * @return
 *      void
 */
void bytebeam_log_level_set(bytebeam_log_level_t level);

/**
 * @brief Get the bytebeam log level
 *
 * @param
 *      void
 * 
 * @return
 *      bytebeam log level
 */
bytebeam_log_level_t bytebeam_log_level_get(void);

/**
 * @brief Set the bytebeam log stream name
 *
 * @param[in] stream_name name of the log stream
 * 
 * @return
 *      BB_SUCCESS: If the log stream was successfully set
 *      BB_FAILURE: If the log stream size exceeded buffer size
 *      BB_NULL_CHECK_FAILURE: If the bytebeam_client is NULL
 */
bytebeam_err_t bytebeam_log_stream_set(char* stream_name);

/**
 * @brief Get the bytebeam log stream name
 *
 * @param
 *      void
 * 
 * @return
 *      bytebeam log stream name
 */
char* bytebeam_log_stream_get();

/**
 * @brief Publish Log to Bytebeam
 *
 * @note  This api works on bytebeam log client handle so make sure to set the bytebeam log handle
 *        before calling this api, If called without setting it will return BB_BB_FAILURE
 * 
 * @param[in] level     indicates log level
 * @param[in] tag       indicates log level
 * @param[in] fmt       variable arguments
 * 
 * @return
 *      BB_SUCCESS : Log publish successful
 *      BB_FAILURE : Log publish failed
 */
bytebeam_err_t bytebeam_log_publish(const char *level, const char *tag, const char *fmt, ...);

#endif /* BYTEBEAM_LOG_H */