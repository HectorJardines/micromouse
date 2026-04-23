#include "log.h"
#include "uart.h"
#include "assert_handler.h"
#include "../external/printf/printf.h"
#include <stdarg.h>

/************* STATIC DECLARATIONS ************/

struct log_handle_t {
    log_level_e level;
};
static struct log_handle_t logger = {.level = LOG_LEVEL_ALL};

static uint32_t msg_len(char *msg);

/************** PUBLIC APIs *************/

static bool initalized = false;
void log_init(void) {
    ASSERT(!initalized, ASSERT_APP_LEVEL);
    usart_init();
    initalized = true;
}

void log_message_f(log_level_e level, char *fmt, ...) {
    if (level != logger.level && logger.level != LOG_LEVEL_ALL)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void log_message(log_level_e level, char *msg, int32_t num) {
    uint32_t len = msg_len(msg);
    usart_write_it(msg, len);
}

void log_set_level(log_level_e level) {
    logger.level = level;
}


/*************** STATIC DEFINITIONS *****************/
static uint32_t msg_len(char *msg) {
    if (msg == NULL)
        return 0;

    uint32_t len;
    while (*msg != '\0') {
        msg++;
        len++;
    }
    return len;
}
