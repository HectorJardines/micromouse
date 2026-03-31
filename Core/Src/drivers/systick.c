#include "../../inc/drivers/systick.h"
#include "../../inc/drivers/encoder.h"
#include "stm32f1xx.h"
#include "../../inc/common/defines.h"
#include "../../inc/drivers/pid.h"
#include "../inc/common/assert_handler.h"

static uint8_t systick_configure(uint32_t ticks) {
    if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
    {
        return (1UL);                                                   /* Reload value impossible */
    }

    SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
    return 0;
}

static uint8_t initialized = 0;
// could just use the systick config function, but i like to see what is happening so copy code here
void systick_init(void) {
    ASSERT(!initialized, ASSERT_PERIPHERAL_LEVEL);
    systick_configure(SYSCLK_FREQ_HZ / 1000);
    initialized = 1;
}

void SysTick_Handler(void) {
    // pid handling every ms ensures our control is system is updated frequently
    update_pid();

    if (encoder_read_left_count() > 31000 || encoder_read_right_count() > 31000
        || encoder_read_left_count() < -31000 || encoder_read_right_count() < -31000) {
            int16_t difference = encoder_read_left_count() - encoder_read_right_count();
            encoder_reset_all();
            encoder_set_count(encoder_left, difference);
    }
}