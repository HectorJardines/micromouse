#ifndef _DMA_H
#define _DMA_H

#include "io.h"

void dma_init(void);
void dma_adc_transfer_control(uint8_t EnOrDi);

#endif