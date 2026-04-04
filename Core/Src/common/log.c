#include "log.h"
#include "uart.h"
#include "assert_handler.h"
#include "../external/printf/printf.h"
#include <stdarg.h>

#define LOG(fmt, ...)

static bool initalized = false;
void log_init(void) {
    ASSERT(!initalized, ASSERT_APP_LEVEL);
    usart_init();
    initalized = true;
}

void log_message(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
