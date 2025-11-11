#ifndef _PWM_H
#define _PWM_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_tim.h"
#include "./io.h"

void pwm_configure(void);
void pwm_set_duty_cycle(uint32_t duty);

#endif