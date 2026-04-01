#include "../inc/common/assert_handler.h"
#include "../inc/drivers/io.h"
#include "../inc/drivers/uart.h"
#include "../inc/main.h"
#include "../external/printf/printf.h"

void test_setup(void) {
    SystemClock_Config();
    io_configure();
}

void test_uart(void) {
    usart_init();
    
    while(1) {
        _putchar('H');
        _putchar('e');
        _putchar('l');
        _putchar('l');
        _putchar('o');
        _putchar('\n');
        HAL_Delay(500);
    }
}

int main(void) {
    test_setup();
    test_uart();
}