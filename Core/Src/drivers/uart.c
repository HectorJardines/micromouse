#include "../inc/drivers/uart.h"
#include "../inc/common/defines.h"
#include "../inc/common/ring_buffer.h"

#define USART_RING_BUF_CAP (32U)
// TX baudrate = f_clk / (16 * USARTDIV)
// 115200 = 32MHz / (16 * UDIV) => USARTDIV = 32MHz / ( 16 * 115200 )
// USARTDIV = 17.36  
// BRR value won't change, instead of using expensive division, just hard code the mantissa/frac
#define USART_BRR_MANTISSA  (17U)
// BRR_FRAC = ceil(16 * .36)
#define USART_BRR_FRAC      (6U)
#define ONGOING     (1U)
#define COMPLETE    (0U)

STATIC_RING_BUFFER(usart_tx_buf, USART_RING_BUF_CAP, uint8_t);
static volatile uint8_t usart_tx_status = COMPLETE;

static void usart_peripheral_clk_ctl(uint8_t EnOrDi) {
    if (EnOrDi == ENABLE)
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else
        RCC->APB1ENR &= ~(RCC_APB1ENR_USART3EN);
}

static void usart_peripheral_ctl(uint8_t EnOrDi) {
    if (EnOrDi == ENABLE)
        USART3->CR1 |= (USART_CR1_UE);
    else
        USART3->CR1 &= ~(USART_CR1_UE);
}

static void usart_tx_interrupt_enable(void) {
    USART3->CR1 |= USART_CR1_TXEIE;
}

static void usart_tx_interrupt_disable(void) {
    USART3->CR1 &= ~(USART_CR1_TXEIE);
}

static void usart_configure(void) {
    // DISABLE HWFLOWCTRL
    USART3->CR3 &= ~(USART_CR3_CTSE | USART_CR3_RTSE);
    // ENABLE TRANSMITTER and RECEIVER
    USART3->CR1 |= (USART_CR1_TE | USART_CR1_RE);
    // configure 8 bit word length
    USART3->CR1 &= ~(USART_CR1_M);
    // configure 1 stop bit
    USART3->CR2 &= ~(0x3U << USART_CR2_STOP_Pos);
    // disable parity control
    USART3->CR1 &= ~(USART_CR1_PCE);
    // configure baudrate register for 115200 baud rate
    USART3->BRR = (USART_BRR_MANTISSA << USART_BRR_DIV_Mantissa_Pos) | USART_BRR_FRAC;

    // CLEAR BITS FOR ASYNC MODE
    USART3->CR2 &= ~(USART_CR2_LINEN | USART_CR2_CLKEN);
    USART3->CR3 &= ~(USART_CR3_SCEN | USART_CR3_HDSEL | USART_CR3_IREN);
}

void usart_init(void) {
    usart_peripheral_clk_ctl(ENABLE);
    usart_configure();

    // TODO: ITS LIKELY THAT THIS WILL TRIGGER AND INTERRUPT AND POP FROM RB BEFORE SETUP
    // usart_tx_interrupt_enable();
    NVIC_EnableIRQ(USART3_IRQn);

    usart_peripheral_ctl(ENABLE);
}

usart_status_e usart_start_write_it(void) {
    if (!ring_buffer_empty(&usart_tx_buf)) {
        char c = 0;
        ring_buffer_peek_tail(&usart_tx_buf, &c);
        USART3->DR = c;
        usart_tx_status = ONGOING;
    }
    return 0;
}

void _putchar(char c) {
    if (c == '\n')
        _putchar('\r');
    // TRANSMISSION ONGOING
    while (ring_buffer_full(&usart_tx_buf));
    usart_tx_interrupt_disable(); // shared resources with interrupt handler
    ring_buffer_push(&usart_tx_buf, &c);
    if (usart_tx_status == COMPLETE)
        usart_start_write_it();
    usart_tx_interrupt_enable();
}

/************************ INTERRUPT APIs *************************/
static uint8_t usart_get_flag_status(uint32_t flag) {
    if (USART3->SR & flag)
        return ENABLE;
    else
        return DISABLE;
}

void USART3_IRQHandler(void) {
    //TODO: WHY DIDNT TCIE WORK? 
    // element at tail of ring buffer has already been TX
    if (!ring_buffer_empty(&usart_tx_buf))
        ring_buffer_pop(&usart_tx_buf, NULL);
    if (usart_get_flag_status(USART_SR_TXE_Msk))
    {
        // send next byte
        if (ring_buffer_empty(&usart_tx_buf)) {
            usart_tx_status = COMPLETE;
            usart_tx_interrupt_disable();
            return;
        }
        usart_start_write_it();
    }
}

void usart_write(char *msg, uint8_t len) {
    for (uint8_t i = len; i > 0; i--) {
        while (!usart_get_flag_status(USART_SR_TXE)) {}
        USART3->DR = *msg;
        msg++;
    }
}

/************************ ASSERT POLLING APIs ************************/
void usart_assert_init(void) {
    usart_peripheral_clk_ctl(ENABLE);
    usart_configure();
    usart_peripheral_ctl(ENABLE);
}

static void usart_putchar_polling(char c) {
    if (c == '\n')
        usart_putchar_polling('\r');
    while (!usart_get_flag_status(USART_SR_TXE_Msk));
    USART3->DR = c;
}

void usart_log_assert(const char *assert_str) {
    while (*assert_str != '\0') {
        usart_putchar_polling(*assert_str);
        assert_str++;
    }
}
