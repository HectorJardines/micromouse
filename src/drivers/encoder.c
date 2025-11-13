#include "../inc/drivers/encoder.h"
#include <stdbool.h>

#define TIM_MAX_ARR_VALUE       (65535UL)

static void tim_peripheral_clk_enable(void)
{
    // enable peripheral clock for timers 3 and 4
    RCC->APB1ENR |= (RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN);
}

static bool initialized = false;
void encoder_init(void)
{
    if (initialized) {
        return;
    }
    // utilize timer 3 and timer 4
    tim_peripheral_clk_enable();
    // select counting on TI1 and TI2 edges
    TIM3->SMCR |= (0x3U << TIM_SMCR_SMS_Pos);
    TIM4->SMCR |= (0x4U << TIM_SMCR_SMS_Pos);
    // set capture compare channel 1 of TIM3/4 as input with Input Capture mapped to TI1
    TIM3->CCMR1 |= ((0x1U << TIM_CCMR1_CC1S_Pos) | (0x1U << TIM_CCMR1_CC2S_Pos));
    TIM4->CCMR1 |= ((0x1U << TIM_CCMR1_CC1S_Pos) | (0x1U << TIM_CCMR1_CC2S_Pos));
    // select TI1 and TI2 polarity
    TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
    TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
    // set ARR to MAX 16 BIT VALUE 
    TIM3->ARR = TIM_MAX_ARR_VALUE;
    TIM4->ARR = TIM_MAX_ARR_VALUE;
    // enable counter
    TIM3->CR1 |= (TIM_CR1_CEN);
    TIM4->CR1 |= (TIM_CR1_CEN);
}

int16_t encoder_read_left_count(void)
{
    return ((int16_t)TIM3->CNT);
}

int16_t encoder_read_right_count(void)
{
    return ((int16_t)TIM4->CNT);
}