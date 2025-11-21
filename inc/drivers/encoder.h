#ifndef _ENCODER_H
#define _ENCODER_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_tim.h"

typedef enum {
    encoder_left,
    encoder_right
} encoder_e;

void encoder_configure(void);
int16_t encoder_read_left_count(void);
int16_t encoder_read_right_count(void);
void encoder_set_count(encoder_e encoder, int16_t count);

#endif