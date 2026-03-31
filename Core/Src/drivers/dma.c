#include "../inc/drivers/dma.h"
#include "../inc/common/assert_handler.h"

#define DMA_XSIZE_16        (1U)
#define DMA_NUM_BYTES_TO_TX (8U) // 2 bytes per ADC channel (4)

static inline void dma_peripheral_clk_enable(void) {
    RCC->APB1ENR |= (RCC_AHBENR_DMA1EN);
}

static void dma_configure_adc_transfer(void) {
    // set 16-bit size memory
    DMA1_Channel1->CCR |= (DMA_XSIZE_16 << DMA_CCR_MSIZE_Pos);
    // set 16-bit peripheral data size
    DMA1_Channel1->CCR |= (DMA_XSIZE_16 << DMA_CCR_PSIZE_Pos);
    // configure DMA circular transfer mode
    DMA1_Channel1->CCR |= (DMA_CCR_CIRC);
    // configure DMA read from peripheral
    DMA1_Channel1->CCR &= ~(DMA_CCR_DIR);

    // configure number of bytes per DMA rotation
    DMA1_Channel1->CNDTR = ((DMA_NUM_BYTES_TO_TX & 0x0FFFF));
    // set peripheral address to ADC1 data register
    DMA1_Channel1->CPAR = ADC1->DR;
    // set memory address

}

static bool initialized = false;
void dma_init(void) {
    ASSERT(!initialized, ASSERT_PERIPHERAL_LEVEL);
    dma_peripheral_clk_enable();
    dma_configure_adc_transfer();
    initialized = true;
}

void dma_adc_transfer_control(uint8_t EnOrDi) {
    if (EnOrDi == ENABLE)
        DMA1_Channel1->CCR |= (DMA_CCR_EN);
    else
        DMA1_Channel1->CCR &= ~(DMA_CCR_EN);
}

void dma_spi_transfer_control(uint8_t EnOrDi) {
    if (EnOrDi == ENABLE)
        DMA1_Channel2->CCR |= (DMA_CCR_EN);
    else
        DMA1_Channel2->CCR &= ~(DMA_CCR_EN);
}