#include "../inc/drivers/spi.h"
#include "../inc/drivers/io.h"
#include "../inc/common/assert_handler.h"
#include <stdbool.h>
#include "log.h"

#define DUMMY_BYTE 0xFFU
#define PB12_BSRR_SET   (0x1U << 12)
#define PB12_BSRR_RESET (0x1U << 28)
#define CS_LOW()    (GPIOB->BSRR |= PB12_BSRR_RESET)
#define CS_HIGH()   (GPIOB->BSRR |= PB12_BSRR_SET)

static inline uint8_t spi_get_flag_status(uint8_t flag) {
    return (SPI2->SR & flag) ? SET : RESET;
}

static inline void spi_enable(void) {
    SPI2->CR1 |= (SPI_CR1_SPE);
}

static void spi_disable(void) {
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
    // define serial clock baud rate (BR[2:0]) want 1MHz; div f_pclk by 32
    SPI2->CR1 |= (0x4U << SPI_CR1_BR_Pos);
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
    // disable software target management
    SPI2->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);
    // set SS on enable, drives NSS low when SPI->SPE is set; HIGH when reset
    SPI2->CR2 &= ~(SPI_CR2_SSOE);
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
    unsigned char dummy_read = 0;
    while (!spi_get_flag_status(SPI_SR_TXE));
    SPI2->DR = *reg_addr;
    while (spi_get_flag_status(SPI_SR_BSY));
    while (!spi_get_flag_status(SPI_SR_RXNE));
    dummy_read = (unsigned char)SPI2->DR;
}

int8_t spi_read_data(unsigned char dev_id, unsigned char reg_addr, unsigned char *data_rd, short unsigned int len) {
    // dev_id is used in BMI160 i2c interface
    CS_LOW();
    // delay to ensure that CS is LOW before any transaction
    for (int i = 0; i < 1000; i++);
    (void)dev_id;
    // unsigned char dummy_read = 0;
    spi_send_addr(&reg_addr);

    for (uint32_t i = len; i > 0; --i) {
        // while (spi_get_flag_status(SPI_SR_BSY));
        while (!spi_get_flag_status(SPI_SR_TXE));
        SPI2->DR = (unsigned char) DUMMY_BYTE;
        while (!spi_get_flag_status(SPI_SR_RXNE));
        *data_rd = (unsigned char)SPI2->DR;
        data_rd++;
    }

    // wait for BSY bit to be cleared and TXE to be set, TX complete
    while (!spi_get_flag_status(SPI_SR_TXE));
    while (spi_get_flag_status(SPI_SR_BSY));
    CS_HIGH();
    return SPI_RES_OK;
}

int8_t spi_write_data(unsigned char dev_id, unsigned char reg_addr, unsigned char *data_wr, short unsigned int len) {
    // dev_id is used in BMI160 i2c interface
    CS_LOW();
    // delay to ensure that CS is LOW before any transaction
    for (int i = 0; i < 1000; i++);
    (void)dev_id;
    // spi_send_addr(&reg_addr);
    while(!spi_get_flag_status(SPI_SR_TXE));
    SPI2->DR = reg_addr;
    for (uint32_t i = len; i > 0; --i) {
        // wait until DR is empty, i.e. data in shift register
        while (!spi_get_flag_status(SPI_SR_TXE));
        // reg_addr has MSB cleared by BMI160 API when passed to write function
        SPI2->DR = (*data_wr);
        data_wr++;
    }
    // wait for BSY bit to be cleared and TXE to be set, TX complete
    while (!spi_get_flag_status(SPI_SR_TXE));
    while (spi_get_flag_status(SPI_SR_BSY));
    CS_HIGH();
    return SPI_RES_OK;
}