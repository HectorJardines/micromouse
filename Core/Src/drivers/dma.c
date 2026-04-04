#include "../inc/drivers/dma.h"
#include "../inc/common/assert_handler.h"

#define DMA_XSIZE_16        (1U)
#define DMA_NUM_BYTES_TO_TX (4U) // 2 bytes per DMA transfer

static inline void dma_peripheral_clk_enable(void) {
    RCC->APB1ENR |= (RCC_AHBENR_DMA1EN);
}

static void dma_transfer_complete_enable(void) {
    DMA1_Channel1->CCR |= (DMA_CCR_TCIE);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void dma_transfer_complete_disable(void) {
    DMA1_Channel1->CCR &= ~(DMA_CCR_TCIE);
    NVIC_DisableIRQ(DMA1_Channel1_IRQn);
}

/**
 * @brief configure dma peripheral for adc peripheral data transfer to memory
 * 
 * COnfigures the DMA peripheral for circular, repeated transfer of adc samples 
 * from the adc peripheral to memory. The transfers are interrupt driven.
 * 
 * @param dma_src_addr 32-bit address corresponding to the adc peripheral data register
 * @param dma_dst_addr 32-bit address corresponding to block of memory where adc samples will be stored and retrired from
 * @return void
 */
static void dma_configure_adc_transfer(uint32_t dma_src_addr, uint32_t dma_dst_addr) {
    // set 16-bit size memory
    DMA1_Channel1->CCR |= (DMA_XSIZE_16 << DMA_CCR_MSIZE_Pos);
    // set 16-bit peripheral data size
    DMA1_Channel1->CCR |= (DMA_XSIZE_16 << DMA_CCR_PSIZE_Pos);
    // configure DMA circular transfer mode
    DMA1_Channel1->CCR |= (DMA_CCR_CIRC);
    // configure DMA read from peripheral
    DMA1_Channel1->CCR &= ~(DMA_CCR_DIR);
    //enable memory increment mode
    DMA1_Channel1->CCR |= (DMA_CCR_MINC);
    // configure TRANSFER COMPLETE interrupts
    dma_transfer_complete_enable();

    // configure number of bytes per DMA rotation
    DMA1_Channel1->CNDTR = ((DMA_NUM_BYTES_TO_TX & 0x0FFFF));
    // set peripheral address to ADC1 data register
    DMA1_Channel1->CPAR = dma_src_addr;
    // set memory address
    DMA1_Channel1->CMAR = dma_dst_addr;
}

static bool initialized = false;
void dma_init(uint32_t dma_src_addr, uint32_t dma_dst_addr) {
    ASSERT(!initialized, ASSERT_PERIPHERAL_LEVEL);
    dma_peripheral_clk_enable();
    dma_configure_adc_transfer(dma_src_addr, dma_dst_addr);
    initialized = true;
}

void dma_set_dst_address(uint32_t dma_dst_addr) {
    // should only try to reset destination address on transfer complete
    ASSERT((DMA1->ISR & DMA_ISR_TCIF1_Msk), ASSERT_PERIPHERAL_LEVEL);
    DMA1_Channel1->CMAR = dma_dst_addr;
}

void dma_clear_tx_complete_int(void) {
    DMA1->IFCR |= (DMA_IFCR_CTCIF1);
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
