#ifndef _ENCODER_H
#define _ENCODER_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_tim.h"

void encoder_configure(void);
int16_t encoder_read_left_count(void);
int16_t encoder_read_right_count(void);

#endif