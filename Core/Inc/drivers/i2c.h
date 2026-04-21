#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>

/*********************
 *      I2C ENUMS
 *********************/
typedef enum {
    I2C_OK,
    I2C_ERR_TX,
    I2C_ERR_RX,
    I2C_BUSY_IN_TX,
    I2C_BUSY_IN_RX
} i2c_status_e;


/**********************
 *    PUBLIC APIs
 **********************/

/**
 * @brief Initialize the I2C peripheral
 * 
 * @return void
 */
void i2c_init(void);

/**
 * @brief I2C blocking transmit API
 * 
 * The blocking implementation seemed more suitable for the SSD1306 use case.
 * At least in configuration of the OLED driver. For writing data to GDDRAM interrupt 
 * based implementation could be useful.
 * 
 * @param[in] data - data buffer to be transmitted
 * @param[in] len - length of data buffer to be transmitted
 */
i2c_status_e i2c_transmit_poll(uint8_t *data, uint32_t len);

/**
 * @brief I2C transmit API to interface with SD1306 oled display driver
 * 
 * 
 * @return i2c_status_e - 0 if success >0 otherwise
 * @note Decided against using an interrupt based approach, adds 0 benefit (we need to poll between writes)
 */
i2c_status_e i2c_transmit(uint8_t *data, uint32_t len);

/**
 * @brief I2C receive API to interface with SD1306 oled display driver
 * 
 * 
 * @return i2c_status_e - 0 if succes >0 otherwise
 */
i2c_status_e i2c_receive(uint8_t *data, uint32_t len);

/**
 * @brief Sets the target addr of which the peripheral is to communicate with
 * 
 * @return void
 */
void i2c_set_dev_addr(uint8_t dev_addr);

#endif