#ifndef _LOG_H
#define _LOG_H

#include "uart.h"

#ifndef DISABLE_FORMAT_LOGGING
#define LOG(log_level, fmt, ...) log_message_f(log_level, "%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(log_level, msg, num) log_message(log_level, msg, num)
#endif

typedef enum {
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_ALL
} log_level_e;

#ifndef DISABLE_LOG
/**
 * @brief Initializes the logging functionality and required UART driver
 */
void log_init(void);

/**
 * @brief Serially logs the specified message, API takes variable arguments like printf
 * 
 * The caller must specify the log level at which the API will be executed.
 * The level parameter allows the user to have log message output only when the user has
 * set the log modules level to match the specified one.
 * 
 * @param message_fmt[in] C format string used for the log message
 * @param args[in] arguments that are to be passed to the format string
 * @return void
 */
void log_message_f(log_level_e level, char *fmt, ...);

/**
 * @brief do reduce memory usage, this API may be used. Does not support printf like logging.
 * 
 * 
 * 
 * @param level level at which this call will be executed
 * @param msg msg string that is to be output serially
 * @param num an optional number parameter (set to -1 if not needed)
 */
void log_message(log_level_e level, char *msg, int32_t num);

/**
 * @brief This API sets the LEVEL of log commands that are executed
 * 
 * This API sets a global log level for all log_message API calls.
 * I.e. only log_message calls that are of the same level as the currently
 * set log level will be executed. E.g. if the current log level is set to 
 * LOG_LEVEL_ERR, only log_message calls that specify the LOG_LEVEL_ERR level
 * will be executed.
 * 
 * @param[in] level the level to set
 */
void log_set_level(log_level_e level);

#else
#define log_init() ;
#define log_message(fmt, ...) ;
#endif

#endif