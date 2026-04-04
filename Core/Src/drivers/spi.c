#include "../inc/drivers/spi.h"
#include "../inc/drivers/io.h"
#include "../inc/common/assert_handler.h"
#include <stdbool.h>

#define DUMMY_BYTE 0xFFU

static inline uint8_t spi_get_flag_status(uint8_t flag) {
    return (SPI2->SR & flag) ? SET : RESET;
}

static inline void spi_enable(void) {
    SPI2->CR1 |= (SPI_CR1_SPE);
}

static void spi_disable(void) {
    // SPI disable procedure for full-duplex TX/RX
    while(!spi_get_flag_status(SPI_SR_TXE));
    while(spi_get_flag_status(SPI_SR_BSY));
    SPI2->CR1 &= ~(SPI_CR1_SPE);
}

static void spi_peripheral_clock_ctrl(uint8_t EnOrDi) {
    if (EnOrDi == ENABLE)
        RCC->APB1ENR |= (RCC_APB1ENR_SPI2EN);
    else
        RCC->APB1ENR &= ~(RCC_APB1ENR_SPI2EN);
}

static void spi_configure(void)
{
    // disable SPI peripheral
    SPI2->CR1 &= ~(SPI_CR1_SPE);
    // define serial clock baud rate (BR[2:0]) want 1MHz
    SPI2->CR1 |= (0x6U << SPI_CR1_BR_Pos);
    // set CPOL and CPHA "11 Mode" for BMI160
    SPI2->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
    // configure full-duplex (2-line unidirection data mode)
    SPI2->CR1 &= ~(SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);
    SPI2->CR1 &= ~(SPI_CR1_RXONLY);
    // set 8-bit data frame format corresponding to BMI160
    SPI2->CR1 &= ~(SPI_CR1_DFF);
    // define MSBFIRST frame format
    SPI2->CR1 &= ~(SPI_CR1_LSBFIRST);
    // set MSTR bit
    SPI2->CR1 |= (SPI_CR1_MSTR);
    spi_enable();
}

static bool intialized = false;
void spi_init(void)
{
    ASSERT(!intialized, ASSERT_PERIPHERAL_LEVEL);
    spi_peripheral_clock_ctrl(ENABLE);
    spi_configure();
    intialized = true;
}

static void spi_send_addr(uint8_t *reg_addr) {
    while (!spi_get_flag_status(SPI_SR_TXE));
    SPI2->DR = *reg_addr;
}

void spi_read_data(uint8_t dev_id, uint8_t reg_addr, uint8_t *data_rd, uint32_t len) {
    // dev_id is used in BMI160 i2c interface
    (void)dev_id;
    uint8_t dummy_read = 0;
    spi_send_addr(&reg_addr);

    for (uint32_t i = len; i > 0; --i) {
        // wait until DR is empty, i.e. data in shift register
        while (!spi_get_flag_status(SPI_SR_TXE));
        // send dummy byte to move shift register
        SPI2->DR = DUMMY_BYTE;

        while (!spi_get_flag_status(SPI_SR_RXNE));
        // clear junk byte from address write
        if (i == len)
            dummy_read = (uint8_t)SPI2->DR;
        else {
            *data_rd = (uint8_t)SPI2->DR;
            data_rd++;
        }
    }
    // read final data byte
    while (!spi_get_flag_status(SPI_SR_RXNE));
    *data_rd = (uint8_t)SPI2->DR;

    // wait for BSY bit to be cleared and TXE to be set, TX complete
    while (!spi_get_flag_status(SPI_SR_TXE));
    while (spi_get_flag_status(SPI_SR_BSY));
}

void spi_write_data(uint8_t dev_id, uint8_t reg_addr, uint8_t *data_wr, uint32_t len) {
    // dev_id is used in BMI160 i2c interface
    (void)dev_id;
    spi_send_addr(&reg_addr);
    for (uint32_t i = len; len > 0; --len) {
        // wait until DR is empty, i.e. data in shift register
        while (!spi_get_flag_status(SPI_SR_TXE));
        // reg_addr has MSB cleared by BMI160 API when passed to write function
        SPI2->DR = (*data_wr);
        data_wr++;
    }
    // wait for BSY bit to be cleared and TXE to be set, TX complete
    while (!spi_get_flag_status(SPI_SR_TXE));
    while (spi_get_flag_status(SPI_SR_BSY));
}