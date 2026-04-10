#ifndef _LOG_H
#define _LOG_H

#include "uart.h"

#define LOG(fmt, ...) log_message("%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#ifndef DISABLE_LOG
/**
 * @brief Initializes the logging functionality and required UART driver
 */
void log_init(void);

/**
 * @brief private function called only in the LOG macro wrapper
 * 
 * 
 * 
 * @param message_fmt[in] C format string used for the log message
 * @param args[in] arguments that are to be passed to the format string
 * @return void
 */
void log_message(char *fmt, ...);

#else
#define log_init() ;
#define log_message(fmt, ...) ;
#endif

#endif