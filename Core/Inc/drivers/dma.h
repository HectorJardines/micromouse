#ifndef _DMA_H
#define _DMA_H

#include "io.h"

/**
 * @brief Initializes the DMA1 peripheral's Channel1 for continuous transfer
 * 
 * @param dma_src_addr address from which samples should be read
 * @param dma_dst_addr address to which samples should be transferred
 */
void dma_init(uint32_t dma_src_addr, uint32_t dma_dst_addr);

/**
 * @brief configure the dma transfer destination address
 * 
 * @param dma_dst_addr 32-bit destination addres
 */
void dma_set_dst_address(uint32_t dma_dst_addr);

/**
 * @brief clear the tx complete interrupt flag for DMA1_Channel1
 */
void dma_clear_tx_complete_int(void);

void dma_adc_transfer_control(uint8_t EnOrDi);

#endif