#include "../../inc/drivers/adc.h"
#include "../inc/common/assert_handler.h"
#include "../Inc/drivers/dma.h"

#define NUM_OF_CONV_PER_SEQ (4U) // REGISTER VALUE STARTS AT 0x00 = 1 conv
#define ADC1_CHAN_4 (4U)
#define ADC1_CHAN_5 (5U)
#define ADC1_CHAN_8 (8U)
#define ADC1_CHAN_9 (9U)

#define NO_SAMPLES_READ (1500U)

adc_channel_values_t sensor_samples = {NO_SAMPLES_READ, NO_SAMPLES_READ, NO_SAMPLES_READ, NO_SAMPLES_READ};
// samples cache allows us to keep a stable block of memory from which we can always read
adc_channel_values_t sensor_samples_cache = {NO_SAMPLES_READ, NO_SAMPLES_READ, NO_SAMPLES_READ, NO_SAMPLES_READ};

static void adc_power_on(void) {
    // disable ADC peripheral
    ADC1->CR2 |= (ADC_CR2_ADON);
    HAL_Delay(2);
    ADC1->CR2 |= (ADC_CR2_ADON);
    // delay of 2 clock cycles required before ADC calibration
    // perform peripheral calibration
    // ADC1->CR2 |= (ADC_CR2_CAL);
}

static void adc_set_channel_sequence(void) {
    // set conversion sequence length to 4
    ADC1->SQR1 |= ((NUM_OF_CONV_PER_SEQ - 1) << ADC_SQR1_L_Pos);
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
    // enable ADC peripheral clock
    RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN);
    // configure the peripheral SCAN mode
    ADC1->CR1 |= (ADC_CR1_SCAN);
    // continuous conversion mode
    ADC1->CR2 |= (ADC_CR2_CONT);
    // enable DMA
    ADC1->CR2 |= (ADC_CR2_DMA);
    // data alignment RIGHT
    ADC1->CR2 &= ~(ADC_CR2_ALIGN);

    // SET CHANNEL SAMPLING TIMES (13.5 Cycles for each channel)
    ADC1->SMPR2 |= ((4U << ADC_SMPR2_SMP4_Pos) | (4U << ADC_SMPR2_SMP5_Pos) | (4U << ADC_SMPR2_SMP8_Pos) | (4U << ADC_SMPR2_SMP9_Pos));

    // configure conversion sequence
    adc_set_channel_sequence();
    dma_init((uint32_t)&ADC1->DR, (uint32_t)&sensor_samples[0]);
    dma_adc_transfer_control(ENABLE);
    adc_power_on();
    initialized = true;
}

void adc_sample_channels(adc_channel_values_t values) {
    // TODO: DISABLE INTERRUPTS, DONT WANT TO MODIFY SAMPLES AS WE COPY OVER
    __disable_irq();
    for (int i = 0; i < NUM_OF_CONV_PER_SEQ; ++i) {
        values[i] = sensor_samples_cache[i];
    }
    __enable_irq();
}

void DMA1_Channel1_IRQHandler(void) {
    // 1. move samples from dma buffer to samples cache
    for (int i = 0; i < NUM_OF_CONV_PER_SEQ; i++)
        sensor_samples_cache[i] = sensor_samples[i];
    // 2. clear interrupt
    dma_clear_tx_complete_int();
}
