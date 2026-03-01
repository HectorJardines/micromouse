#ifndef _ADC_H
#define _ADC_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_adc.h"
#include <stdint.h>

#define ADC_CHANNEL_CNT (16U)
#define ADC_IRQ_NO      (0U)

typedef uint16_t adc_channel_values_t[ADC_CHANNEL_CNT];

void adc_init(void);
void adc_sample_channels(adc_channel_values_t *values);

#endif