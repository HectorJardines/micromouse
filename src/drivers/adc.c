#include "../../inc/drivers/adc.h"

#define NUM_OF_CONV_PER_SEQ (4U)
#define ADC1_CHAN_4 (4U)
#define ADC1_CHAN_5 (5U)
#define ADC1_CHAN_8 (8U)
#define ADC1_CHAN_9 (9U)

static void adc_power_on(void) {
    // enable ADC peripheral clock
    RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN);
    // disable ADC peripheral
    ADC1->CR2 |= (ADC_CR2_ADON);
    // delay of 2 clock cycles required before ADC calibration
    HAL_Delay(1);
    // perform peripheral calibration
    ADC1->CR2 |= (ADC_CR2_CAL);
}

static void adc_set_channel_sequence(void) {
    // set conversion sequence length to 4
    ADC1->SQR1 |= (NUM_OF_CONV_PER_SEQ << ADC_SQR1_L_Pos);
    // set channels 4, 5, 8, 9 as channels in sequence
    ADC1->SQR3 |= (ADC1_CHAN_4 << ADC_SQR3_SQ1_Pos);
    ADC1->SQR3 |= (ADC1_CHAN_5 << ADC_SQR3_SQ2_Pos);
    ADC1->SQR3 |= (ADC1_CHAN_8 << ADC_SQR3_SQ3_Pos);
    ADC1->SQR3 |= (ADC1_CHAN_9 << ADC_SQR3_SQ4_Pos);
}

static bool initialized = false;
void adc_init(void) {
    ASSERT(!initialized);
    adc_power_on();
    // configure the peripheral SCAN mode
    ADC1->CR1 |= (ADC_CR1_SCAN);
    // continuous conversion mode
    ADC1->CR2 |= (ADC_CR2_CONT);
    // enable DMA
    ADC1->CR2 |= (ADC_CR2_DMA);
    // data alignment RIGHT
    ADC1->CR2 &= ~(ADC_CR2_ALIGN);
    // configure conversion sequence
    adc_set_channel_sequence();
    initialized = true;
}

void adc_sample_channels(adc_channel_values_t *values) {
    
}
