#include "../inc/drivers/encoder.h"
#include <stdbool.h>

#define TIM_MAX_ARR_VALUE       (65535UL)

TIM_TypeDef *encoder_configs[] = {
    [encoder_left] = TIM3,
    [encoder_right] = TIM4
};

static void tim_peripheral_clk_enable(void)
{
    // enable peripheral clock for timers 3 and 4
    RCC->APB1ENR |= (RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN);
}

static void encoder_configure(encoder_e encoder) {
    // select counting on TI1 and TI2 edges
    encoder_configs[encoder]->SMCR |= (0x3U << TIM_SMCR_SMS_Pos);
    // set capture compare channel 1 of TIM3/4 as input with Input Capture mapped to TI1
    encoder_configs[encoder]->CCMR1 |= ((0x1U << TIM_CCMR1_CC1S_Pos) | (0x1U << TIM_CCMR1_CC2S_Pos));
    // select TI1 and TI2 polarity
    encoder_configs[encoder]->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
    // set ARR to MAX 16 BIT VALUE 
    encoder_configs[encoder]->ARR = TIM_MAX_ARR_VALUE;
    // enable counter
    encoder_configs[encoder]->CR1 |= (TIM_CR1_CEN);
}

static bool initialized = false;
void encoder_init(void)
{
    if (initialized) {
        return;
    }
    // utilize timer 3 and timer 4
    tim_peripheral_clk_enable();
    // configure timers 3 and 4 in encoder interface mode
    encoder_configure(encoder_left);
    encoder_configure(encoder_right);
    initialized = true;
}

int16_t encoder_read_left_count(void)
{
    return ((int16_t)TIM3->CNT);
}

int16_t encoder_read_right_count(void)
{
    return ((int16_t)TIM4->CNT);
}

void encoder_set_count(encoder_e encoder, int16_t count) {
    encoder_configs[encoder]->CNT = count;
}

void encoder_reset_all(void) {
    encoder_configs[encoder_left] = 0;
    encoder_configs[encoder_right] = 0;
}