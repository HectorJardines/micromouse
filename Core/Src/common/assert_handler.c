#include "../inc/common/assert_handler.h"
#include "../inc/drivers/io.h"
#include "../inc/drivers/uart.h"
#include "../external/printf/printf.h"

#define IO_OUT_MODE_FAST    0x3U
#define IO_ALT_FUN_PP       0x2U
#define ASSERT_STR_MAX      (16U + 8U + 1U)
#define BREAKPOINT __asm volatile ("bkpt #0")

static void assert_log(uint32_t pc_val) {
    // configure PB10 as our USART2_TX pin to log the ASSERT location and cause
    usart_assert_init();
    GPIOB->CRH |= (IO_OUT_MODE_FAST << GPIO_CRH_MODE10_Pos);
    GPIOB->CRH |= (IO_ALT_FUN_PP << GPIO_CRH_CNF10_Pos);
    
    char assert_string[ASSERT_STR_MAX];
    snprintf(assert_string, sizeof(assert_string), "ASSERT 0x%x\n", pc_val);
    usart_log_assert(assert_string);
}

static void assert_blink(assert_type_e type) {
    io_e assert_pin = 0;
    switch(type) {
        case ASSERT_APP_LEVEL:
            //toggle red led
            assert_pin = IO_LED_RED;
            break;
        case ASSERT_DRIVER_LEVEL:
            // toggle blue led
            assert_pin = IO_LED_BLUE;
            break;
        case ASSERT_PERIPHERAL_LEVEL:
            // toggle green led
            assert_pin = IO_LED_GREEN;
            break;
    }

    while (1) {
        io_toggle_out(assert_pin);
        HAL_Delay(100);
    }
}

void assert_handler(uint32_t pc_val, assert_type_e type) {
    // BREAKPOINT;
    assert_log(pc_val);
    assert_blink(type);
}