#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>
#include "../inc/drivers/io.h"

#define ADC_ACTIVE_CHANNEL_CNT (4U)
#define ADC_IRQ_NO      (0U)

typedef uint16_t adc_channel_values_t[ADC_ACTIVE_CHANNEL_CNT];

/**
 * @brief Initialize ADC peripheral for continuous SCAN mode of 4 channels
 */
void adc_init(void);

/**
 * @brief Read cached adc channel samples
 */
void adc_sample_channels(adc_channel_values_t *values);

#endif