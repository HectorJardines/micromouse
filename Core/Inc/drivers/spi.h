#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

typedef enum {
    SPI_RES_OK,
    SPI_RES_ERROR = -1
} spi_res_e;

void spi_init(void);
int8_t spi_read_data(unsigned char dev_id, unsigned char reg_addr, unsigned char *data_rd, short unsigned int len);
int8_t spi_write_data(unsigned char dev_id, unsigned char reg_addr, unsigned char *data_wr, short unsigned int len);

#endif