#include "../inc/common/assert_handler.h"
#include "../inc/drivers/io.h"
#include "../inc/drivers/uart.h"
#include "../inc/main.h"
#include "../external/printf/printf.h"

void test_setup(void) {
    // HAL_MspInit();
    SystemClock_Config();
    io_configure();
}

void test_uart(void) {
    usart_init();
    char msg[7] = "Hello\n";
    while(1) {
        usart_write(msg, 6);
    }
}

void test_putchar(void) {
    usart_init();

    while(1) {
        _putchar('H');
        _putchar('e');
        _putchar('l');
        _putchar('l');
        _putchar('o');
        _putchar('\n');
    }
}

void test_assert(void) {
    ASSERT(0, ASSERT_DRIVER_LEVEL);
}

int main(void) {
    test_setup();
    test_assert();
}