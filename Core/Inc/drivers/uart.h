#ifndef _USART_H
#define _USART_H

#include "../inc/drivers/io.h"
#define USART3_TEST              ((USART_TypeDef *)USART3_BASE)

typedef enum {
    USART_OK,
    USART_TX_ERROR,
    USART_RX_ERROR
} usart_status_e;

/**
 * @brief Initialize USART peripheral for interrupt driven TX/RX.
 * 
 * 
 * 
 * @return void 
 */
void usart_init(void);

/**
 * @brief Serial read over interrupt driven usart peripheral.
 * 
 * @return usart_status_e
 */
usart_status_e usart_read_it(void);

/**
 * @brief Serial write over interrupt driven usart peripheral.
 * 
 * @return usart_status_e 
 */
usart_status_e usart_write_it(void);

/**
 * @brief Initialize USART peripheral for polling based TX.
 * 
 * 
 * @param[in]
 * @return void
 * 
 * @note A polling based TX implementation is preferred since the processor should
 * be halted at the point of failure, i.e. it is impractical to service interrupts here. 
 */
void usart_assert_init(void);

/**
 * @brief Log the point of failure including line number and file name.
 * 
 * @param[in] assert_str const char*
 * @return void
 */
void usart_log_assert(const char *assert_str);

void usart_write(char *msg, uint8_t len);

#endif