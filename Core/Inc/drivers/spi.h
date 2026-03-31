#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

void spi_init(void);
void spi_read_data(uint8_t dev_id, uint8_t reg_addr, uint8_t *data_rd, uint32_t len);
void spi_write_data(uint8_t dev_id, uint8_t reg_addr, uint8_t *data_wr, uint32_t len);

#endif