#include "../../inc/drivers/adc.h"
#include "../inc/common/assert_handler.h"
#include "../Inc/drivers/dma.h"

#define NUM_OF_CONV_PER_SEQ (4U)
#define ADC1_CHAN_4 (4U)
#define ADC1_CHAN_5 (5U)
#define ADC1_CHAN_8 (8U)
#define ADC1_CHAN_9 (9U)

#define NO_SAMPLES_READ (1500U)

adc_channel_values_t sensor_samples = {NO_SAMPLES_READ};
// samples cache allows us to keep a stable block of memory from which we can always read
adc_channel_values_t sensor_samples_cache = {NO_SAMPLES_READ};

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
    ASSERT(!initialized, ASSERT_PERIPHERAL_LEVEL);
    // TODO: CHECK IF WE NEED TO POWER OFF ADC TO CONFIGURE
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
    dma_init(&ADC1->DR, sensor_samples);
    initialized = true;
}

void adc_sample_channels(adc_channel_values_t *values) {
    // TODO: DISABLE INTERRUPTS, DONT WANT TO MODIFY SAMPLES AS WE COPY OVER
    for (int i = 0; i < NUM_OF_CONV_PER_SEQ; ++i) {
        *values[i] = sensor_samples_cache[i];
    }
}

void DMA1_Channel1_IRQHandler(void) {
    // 1. move samples from dma buffer to samples cache
    for (int i = 0; i < NUM_OF_CONV_PER_SEQ; i++)
        sensor_samples_cache[i] = sensor_samples[i];
    // 2. clear interrupt
    dma_clear_tx_complete_int();
}
