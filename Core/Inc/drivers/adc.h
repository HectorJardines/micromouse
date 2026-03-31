#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>
#include "../inc/drivers/io.h"

#define ADC_CHANNEL_CNT (16U)
#define ADC_IRQ_NO      (0U)

typedef uint16_t adc_channel_values_t[ADC_CHANNEL_CNT];

void adc_init(void);
void adc_sample_channels(adc_channel_values_t *values);

#endif