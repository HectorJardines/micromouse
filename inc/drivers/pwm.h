#ifndef _PWM_H
#define _PWM_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_tim.h"
#include "./io.h"

typedef enum {
    PWM_L293D_LEFT,
    PWM_L293D_RIGHT
} pwm_channel_e;

void pwm_init(void);
void pwm_set_duty_cycle(uint32_t duty);
void pwm_enable(pwm_channel_e pwm, uint8_t EnOrDi);

#endif