#include "i2c.h"
#include "../Inc/drivers/io.h"
#include "../Inc/common/log.h"

#define TX_ONGOING  (1U)
#define TX_COMPLETE (0U)
#define RX_ONGOING  (1U)
#define RX_COMPLETE (0U)

#define I2C_READ        (1U)
#define I2C_WRITE       (0U)
#define I2C_INVALID     (2U)
#define I2C_READ_Msk    (0x01)
#define I2C_SHIFT_ADDR  (1U)

#define I2C_RX_MODE_SINGLE_BYTE (0U)
#define I2C_RX_MODE_DOUBLE_BYTE (1U)
#define I2C_RX_MODE_MULTI_BYTE  (2U)

// T_rise max = 300ns 
// T_pclk1 = 31.25ns
// 300ms/31.25 = 9.6 + 1 = 11
#define I2C1_TRISE_VALUE    (11U)
// Tpclk1 = 31.25 ns
// T_high = 2*T_low
// 2 * CCR * T_pclk1 / CRR * T_pclk1 = 2
// CCR * T_pclk1 = 2 * CCR * T_pclk1 => 
// F_target = 3 * CCR * T_pclk1 => CCR = F_pclk1 / 3 * F_target 
#define I2C1_CCR_VALUE      (27U)
/************************
 *  STATIC DECLARATIONS
 ************************/
struct i2c_handle_t {
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    uint16_t tx_len;
    uint16_t rx_len;
    uint8_t target_addr;
    uint8_t tx_status;
    uint8_t rx_status;
    uint8_t rx_mode;
};
static struct i2c_handle_t i2c1_handle = {.rx_buffer = NULL, .tx_buffer = NULL, .rx_len = 0, .tx_len = 0, .target_addr = 0x00U, .tx_status = TX_COMPLETE, .rx_status = RX_COMPLETE};

static void i2c_configure(void);
static void i2c_peri_ctl(uint8_t en_or_di);
static uint8_t i2c_get_flag_status(uint32_t flag);
static uint8_t i2c_exec_addr_phase(uint8_t addr, uint8_t rd_or_wr);
static void i2c_clear_addr_bit(void);

/************************
 * USER DEFINED APIs     
 ************************/

/**
 * @brief
 */
void i2c_init(void) {
    RCC->APB1ENR |= (RCC_APB1ENR_I2C1EN);
    i2c_configure();
}

i2c_status_e i2c_transmit_poll(uint8_t *data, uint32_t len) {
    // generate start:
    I2C1->CR1 |= (I2C_CR1_START);
    // wait for SB
    while (!i2c_get_flag_status(I2C_SR1_SB_Msk));
    // exec address phase
    i2c_exec_addr_phase(i2c1_handle.target_addr, I2C_WRITE);
    while (!i2c_get_flag_status(I2C_SR1_ADDR_Msk));
    i2c_clear_addr_bit();
    for (uint32_t i = 0; i < len; ++i) {
        while (!i2c_get_flag_status(I2C_SR1_TXE_Msk));
        I2C1->DR = (uint8_t)(*data);
        data++;
    }
    while(!i2c_get_flag_status(I2C_SR1_TXE));
    while(!i2c_get_flag_status(I2C_SR1_BTF));
    I2C1->CR1 |= (I2C_CR1_STOP);
    return I2C_OK;
}

/**
 * @brief interrupt based i2c tx API
 */
i2c_status_e i2c_transmit(uint8_t *data, uint32_t len) {
    if (i2c1_handle.tx_status == TX_ONGOING) {
        LOG(LOG_LEVEL_DEBUG, "I2C TRANSMISSION ALREADY ONGOING\n");
        return I2C_BUSY_IN_TX;
    }
    else if (i2c1_handle.rx_status == RX_ONGOING) {
        LOG(LOG_LEVEL_DEBUG, "I2C TX FAILED; I2C BUSY IN RX\n");
        return I2C_BUSY_IN_RX;
    }

    // if we do this even driven...
    // 1. disable interrupts 
    __NVIC_DisableIRQ(I2C1_EV_IRQn);
    // 2. enable acking
    I2C1->CR1 |= (I2C_CR1_ACK);
    // 3. Generate Start Condition
    I2C1->CR1 |= (I2C_CR1_START);
    // 3. store data ptr and len in handler
    i2c1_handle.tx_buffer = data;
    i2c1_handle.tx_len = len;
    // 4. set i2c handle busy in TX
    i2c1_handle.tx_status = TX_ONGOING;
    // 5. enable interrupts
    __NVIC_EnableIRQ(I2C1_EV_IRQn);
    return I2C_OK;
}

/**
 * @brief
 */
i2c_status_e i2c_receive(uint8_t *data, uint32_t len) {
    if (i2c1_handle.rx_status == RX_ONGOING) {
        LOG(LOG_LEVEL_DEBUG, "I2C RECEPTION ALREADY ONGOING\n");
        return I2C_BUSY_IN_RX;
    }
    else if (i2c1_handle.tx_status == TX_ONGOING) {
        LOG(LOG_LEVEL_DEBUG, "I2C RX FAILED; I2C BUSY IN TX\n");
        return I2C_BUSY_IN_RX;
    }

    __NVIC_DisableIRQ(I2C1_EV_IRQn);
    // three cases: single byte, 2 byte, N>2 bytes
    I2C1->CR1 |= (I2C_CR1_START);
    if (len == 1)
        i2c1_handle.rx_mode = I2C_RX_MODE_SINGLE_BYTE;
    else if (len == 2) {
        i2c1_handle.rx_mode = I2C_RX_MODE_DOUBLE_BYTE;
        I2C1->CR1 |= (I2C_CR1_POS);
    }
    else
        i2c1_handle.rx_mode = I2C_RX_MODE_MULTI_BYTE;

    I2C1->CR1 |= (I2C_CR1_ACK);
    i2c1_handle.rx_buffer = data;
    i2c1_handle.rx_len = len;
    i2c1_handle.rx_status = RX_ONGOING;
    __NVIC_EnableIRQ(I2C1_EV_IRQn);
    return I2C_OK;
}

/**
 * @brief
 */
void i2c_set_dev_addr(uint8_t dev_addr) {
    i2c1_handle.target_addr = dev_addr;
}

/*************************
 *  STATIC DEFINITIONS
 *************************/

static uint8_t i2c_get_flag_status(uint32_t flag) {
    return I2C1->SR1 & flag;
}

static void i2c_peri_ctl(uint8_t en_or_di) {
    I2C1->CR1 |= I2C_CR1_PE;
}

static void i2c_configure(void) {
    // configure 32MHz APB frequency
    I2C1->CR2 |= (0x20U << I2C_CR2_FREQ_Pos);
    // set i2c fast mode
    I2C1->CCR |= (I2C_CCR_FS);
    // set FM duty t_low/t_high = 2
    I2C1->CCR &= ~(I2C_CCR_DUTY);
    I2C1->CCR |= (I2C1_CCR_VALUE << I2C_CCR_CCR_Pos);
    I2C1->TRISE = I2C1_TRISE_VALUE;
    // enable buffer interrupts
    I2C1->CR2 |= (I2C_CR2_ITBUFEN | I2C_CR2_ITEVTEN);
    i2c_peri_ctl(ENABLE);
}

static uint8_t i2c_exec_addr_phase(uint8_t addr, uint8_t rd_or_wr) {
    if (rd_or_wr == I2C_READ) {
        // shift addr and set LSB
        addr = (addr << I2C_SHIFT_ADDR);
        addr |= I2C_READ_Msk;
    }
    else {
        addr = (addr << I2C_SHIFT_ADDR);
    }
    I2C1->DR = (uint8_t)addr;
    return I2C_OK;
}

static void i2c_tx_next_byte(void) {
    I2C1->DR = *((uint8_t *)i2c1_handle.tx_buffer);
    i2c1_handle.tx_buffer++;
    i2c1_handle.tx_len--;
}

static void i2c_rx_next_byte(void) {
    *(i2c1_handle.rx_buffer++) = (uint8_t)I2C1->DR;
    i2c1_handle.rx_len--;
}

static void i2c_clear_addr_bit(void) {
    uint8_t dummy_read = I2C1->SR1;
    dummy_read = I2C1->SR2;
    (void)dummy_read;
}

/**
 * INTERRUPT HANDLING
 */
void I2C1_EV_IRQHandler(void) {
        /********************** EVENT INTERRUPTS *******************/
    if (i2c_get_flag_status(I2C_SR1_SB_Msk)) {
        uint8_t rd_or_wr = i2c1_handle.tx_status == TX_ONGOING ? I2C_WRITE : i2c1_handle.rx_status == RX_ONGOING ? I2C_READ : I2C_INVALID;
        i2c_exec_addr_phase(i2c1_handle.target_addr, rd_or_wr);
    }
    if (i2c_get_flag_status(I2C_SR1_ADDR_Msk)) {
        if (i2c1_handle.tx_status == TX_ONGOING) {
            i2c_clear_addr_bit();
            i2c_tx_next_byte();
        }
        else if (i2c1_handle.rx_status == RX_ONGOING) {
            if (i2c1_handle.rx_mode == I2C_RX_MODE_SINGLE_BYTE) {
                I2C1->CR1 &= ~(I2C_CR1_ACK);
                i2c_clear_addr_bit();
                I2C1->CR1 |= (I2C_CR1_STOP);
            }
            else if (i2c1_handle.rx_mode == I2C_RX_MODE_DOUBLE_BYTE) {
                i2c_clear_addr_bit();
                I2C1->CR1 &= ~(I2C_CR1_ACK);
            }
            else {
                i2c_clear_addr_bit();
            }
        }
    }
    if (i2c_get_flag_status(I2C_SR1_BTF_Msk)) {
        if (i2c1_handle.rx_status == RX_ONGOING && i2c1_handle.rx_mode == I2C_RX_MODE_DOUBLE_BYTE) {
            I2C1->CR1 |= (I2C_CR1_STOP);
            i2c_rx_next_byte();
            i2c_rx_next_byte();
            i2c1_handle.rx_status = RX_COMPLETE;
        }
        if (i2c1_handle.rx_status == RX_ONGOING && i2c1_handle.rx_mode == I2C_RX_MODE_MULTI_BYTE) {
            I2C1->CR1 &= ~(I2C_CR1_ACK);
            i2c_rx_next_byte();
            I2C1->CR1 |= (I2C_CR1_STOP);
            i2c_rx_next_byte();
        }
        if (i2c1_handle.tx_status == TX_ONGOING && i2c1_handle.tx_len == 0) {
            I2C1->CR1 |= (I2C_CR1_STOP);
            i2c1_handle.tx_status = TX_COMPLETE;
            __NVIC_DisableIRQ(I2C1_EV_IRQn);
        }
    }
    
    /********************* BUFFER INTERRUPTS *******************/
    // I2C TXE INTERRUPTS
    if (i2c_get_flag_status(I2C_SR1_TXE_Msk) && i2c1_handle.tx_status == TX_ONGOING) {
        if (i2c1_handle.tx_len > 0)
            i2c_tx_next_byte();
    }
    // I2C RXNE INTERRUPT
    if (i2c_get_flag_status(I2C_SR1_RXNE_Msk)  && i2c1_handle.rx_status == RX_ONGOING) {
        if (i2c1_handle.rx_mode == I2C_RX_MODE_SINGLE_BYTE) {
            i2c_rx_next_byte();
            i2c1_handle.rx_status = RX_COMPLETE;
        }
        else if (i2c1_handle.rx_mode == I2C_RX_MODE_MULTI_BYTE) {
            if (i2c1_handle.rx_len > 3)
                i2c_rx_next_byte();
            else if (i2c1_handle.rx_len == 1) {
                i2c_rx_next_byte();
                i2c1_handle.rx_status = RX_COMPLETE;
            }
        }
    }
}
