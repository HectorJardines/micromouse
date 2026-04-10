#include "../../inc/drivers/io.h"

#define PIN_COUNT           (48U)
#define PORT_COUNT          (3U)
#define IO_CRx_CNF_OFF      (2U)
#define NO_OF_HI_BITS       (8U)
#define NO_BITS_PER_EXTI    (4U)

#define IO_PORT_OFFSET      (4U)
#define IO_PORT_MASK        (0x3U << IO_PORT_OFFSET)
#define IO_PIN_MASK         (0x0F)

#define UNUSED_CONFIG      {io_mode_input, io_cnf_input_analog}
#define RESERVED_CONFIG    {io_mode_reserved, io_cnf_reserved}

io_config_t pin_configurations[PIN_COUNT] = {
    // PORT A
    [IO_UNUSED_0] = UNUSED_CONFIG, [IO_UNUSED_1] = UNUSED_CONFIG, [IO_M1_PWM] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp}, [IO_M2_PWM] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp},
    [IO_IR_RECEIVE_4] = UNUSED_CONFIG, [IO_IR_RECEIVE_3] = UNUSED_CONFIG, [M1_ENC_A] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp}, [M1_ENC_B] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp},
    [M1_DRIVER_IN1] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [IO_IR_EMIT_4] = UNUSED_CONFIG, [IO_IR_EMIT_3] = UNUSED_CONFIG, [IO_IR_EMIT_2] = UNUSED_CONFIG,
    [IO_IR_EMIT_1] = UNUSED_CONFIG, [IO_RESERVED_13] = RESERVED_CONFIG, [IO_RESERVED_14] = RESERVED_CONFIG, [IO_UNUSED_15] = UNUSED_CONFIG,

    // PORT B
    [IO_IR_RECEIVE_2] = UNUSED_CONFIG, [IO_IR_RECEIVE_1] = UNUSED_CONFIG, [IO_UNUSED_18] = UNUSED_CONFIG, [IO_UNUSED_19] = UNUSED_CONFIG,
    [IO_UNUSED_20] = UNUSED_CONFIG, [M2_DRIVER_IN2] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [M2_ENC_B] = {.mode = io_mode_input, .mode_config = io_cnf_input_float}, [M2_ENC_A] = {.mode = io_mode_input, .mode_config = io_cnf_input_float},
    [M2_DRIVER_IN1] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [M1_DRIVER_IN2] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [IO_USART3_TX] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp}, [IO_USART3_RX] = {.mode = io_mode_input, .mode_config = io_cnf_input_float},
    [IO_UNUSED_28] = UNUSED_CONFIG, [SPI2_SCK] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp}, [SPI2_MISO] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp}, [SPI2_MOSI] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_af_pp},

    // PORT C
    [IO_RESERVED_32] = RESERVED_CONFIG, [IO_RESERVED_33] = RESERVED_CONFIG, [IO_RESERVED_34] = RESERVED_CONFIG, [IO_RESERVED_35] = RESERVED_CONFIG,
    [IO_RESERVED_36] = RESERVED_CONFIG, [IO_RESERVED_37] = RESERVED_CONFIG, [IO_RESERVED_38] = RESERVED_CONFIG, [IO_RESERVED_39] = RESERVED_CONFIG,
    [IO_RESERVED_40] = RESERVED_CONFIG, [IO_RESERVED_41] = RESERVED_CONFIG, [IO_RESERVED_42] = RESERVED_CONFIG, [IO_RESERVED_43] = RESERVED_CONFIG,
    [IO_RESERVED_44] = RESERVED_CONFIG, [IO_LED_RED] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [IO_LED_BLUE] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}, [IO_LED_GREEN] = {.mode = io_mode_output_10MHz, .mode_config = io_cnf_output_gpio_pp}
};

static GPIO_TypeDef *gpiox[PORT_COUNT] = { GPIOA, GPIOB, GPIOC };

typedef void (*irq_handler)(void);
static irq_handler interrupt_handlers[PORT_COUNT][PIN_COUNT] = {
    [IO_PORTA_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    [IO_PORTB_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    [IO_PORTC_NUM] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

/************************
 *      IO CONFIG APIs
 ************************/
/**
 * these functions allow user to retrieve the io port, pin number, and pin bit position
 * the stm32f103c8t6 has 48 pins and 3 ports, the io_e enum represents each pin as a 
 * numeric value from 0-47, the lower 4 bits can be used to represent the pin number.
 * the following 2 bits can be used to represent the three ports
*/

static uint8_t io_port(io_e io)
{
    return (io & IO_PORT_MASK) >> IO_PORT_OFFSET;
}

static uint8_t io_pin_idx(io_e io)
{
    return (io & IO_PIN_MASK);
}

static uint8_t io_pin_bit(io_e io)
{
    return 1 << (io & IO_PIN_MASK);
}

void io_toggle_out(io_e io) {
    uint8_t port = io_port(io);
    uint8_t pin_no = io_pin_idx(io);

    gpiox[port]->ODR ^= (0x1U << pin_no);
}

void io_set_out(io_e io, io_out_e out)
{
    uint8_t port = io_port(io);
    uint8_t pin_no = io_pin_idx(io);

    if (out == LOW)
        gpiox[port]->ODR &= ~(0x1U << pin_no);
    else
        gpiox[port]->ODR |= (0x1U << pin_no);
}

bool io_verify_config(io_config_t *curr_cnf, io_config_t *expected_cnf)
{
    return ((curr_cnf->mode == expected_cnf->mode) && (curr_cnf->mode_config == expected_cnf->mode_config));
}

void io_get_configuration(io_e io, io_config_t *actual)
{
    uint8_t port = io_port(io);
    uint8_t pin_idx = io_pin_idx(io);
    uint8_t pin_mode_bit = pin_idx * 4;
    
    if (pin_idx > IO_PIN_7) {
        pin_idx = pin_idx % NO_OF_HI_BITS;
        pin_mode_bit = pin_idx * 4;
        // get pin mode
        actual->mode = (gpiox[port]->CRH & (0x3U << pin_mode_bit)) >> pin_mode_bit;
        // get pin configuration
        actual->mode_config = ((gpiox[port]->CRH & (0x3U << (pin_mode_bit + IO_CRx_CNF_OFF))) >> (pin_mode_bit + IO_CRx_CNF_OFF));
    }
    else {
        // get pin mode
        actual->mode = (gpiox[port]->CRL & (0x3U << pin_mode_bit)) >> pin_mode_bit;
        // get pin configuration
        actual->mode_config = ((gpiox[port]->CRL & (0x3U << (pin_mode_bit + IO_CRx_CNF_OFF))) >> (pin_mode_bit + IO_CRx_CNF_OFF));
    }
}

void io_init(io_e io, const io_config_t *io_config)
{
    uint8_t port = io_port(io);
    uint8_t pin_idx = io_pin_idx(io);
    uint8_t pin_mode_bit = pin_idx * 4;
    
    if (pin_idx > IO_PIN_7) {
        pin_idx = pin_idx % NO_OF_HI_BITS;
        pin_mode_bit = pin_idx * 4;
        // clear pin mode
        gpiox[port]->CRH &= ~(0x3U << (pin_mode_bit));
        // set pin mode
        gpiox[port]->CRH |= (io_config->mode << (pin_mode_bit));
        
        //clear pin cnf
        gpiox[port]->CRH &= ~(0x3U << (pin_mode_bit + IO_CRx_CNF_OFF));
        // set pin configuration
        gpiox[port]->CRH |= (io_config->mode_config << (pin_mode_bit + IO_CRx_CNF_OFF));
    }
    else {
        // clear pin mode
        gpiox[port]->CRL &= ~(0x3U << (pin_mode_bit));
        // set pin mode
        gpiox[port]->CRL |= (io_config->mode << (pin_mode_bit));

        //clear pin cnf
        gpiox[port]->CRL &= ~(0x3U << (pin_mode_bit + IO_CRx_CNF_OFF));
        // set pin configuration
        gpiox[port]->CRL |= (io_config->mode_config << (pin_mode_bit + IO_CRx_CNF_OFF));
    }
}

static void io_enable_ports(void) {
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN);
    RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN);
}

void io_configure(void)
{
    io_enable_ports();
    io_config_t actual_config;
    for (io_e pin_no = 0; pin_no < PIN_COUNT; ++pin_no) {
        // SKIP INITIALIZATION OF RESERVED PINS
        if (pin_configurations[pin_no].mode == io_mode_reserved || pin_configurations[pin_no].mode_config == io_cnf_reserved)
            continue;

        io_init(pin_no, &pin_configurations[pin_no]);
        io_get_configuration(pin_no, &actual_config);
        if (io_verify_config(&actual_config, &pin_configurations[pin_no]) == false) {
            while(1) {
                // code to toggle LED
            }
        }
    }
}

/********************************
 *       IO INTERRUPT APIs
 */
static IRQn_Type get_irq_no(uint8_t io_num) {
    IRQn_Type irq_no = 0;
    switch (io_num) {
        case IO_PIN_0:
            irq_no = EXTI0_IRQn;
            break;
        case IO_PIN_1:
            irq_no = EXTI1_IRQn;
            break;
        case IO_PIN_2:
            irq_no = EXTI2_IRQn;
            break;
        case IO_PIN_3:
            irq_no = EXTI3_IRQn;
            break;
        case IO_PIN_4:
            irq_no = EXTI4_IRQn;
            break;
        case IO_PIN_5:
        case IO_PIN_6:
        case IO_PIN_7:
        case IO_PIN_8:
        case IO_PIN_9:
            irq_no = EXTI9_5_IRQn;
            break;
        case IO_PIN_10:
        case IO_PIN_11:
        case IO_PIN_12:
        case IO_PIN_13:
        case IO_PIN_14:
        case IO_PIN_15:
            irq_no = EXTI15_10_IRQn;
            break;
    }
    return irq_no;
}

static void io_irq_enable_interrupt(IRQn_Type io_irq_no, uint32_t irq_prio) {
    NVIC_EnableIRQ(io_irq_no);
    NVIC_SetPriority(io_irq_no, irq_prio);
}

void register_interrupt(io_e io, irq_handler handler)
{
    io_pin_no_e pin_num = io_pin_idx(io);
    io_port_num_e port_num = io_port(io);
    interrupt_handlers[port_num][pin_num] = handler;
}

void io_interrupt_configure(io_e io, exti_no_e exti_no, io_it_trigger_e trigger)
{
    uint8_t port_num = io_port(io);

    uint8_t exticr_num = exti_no / 4;
    uint8_t exticr_bit_idx = (exti_no % 4) * NO_BITS_PER_EXTI;

    // clear and set port for EXTI line to be configured to
    AFIO->EXTICR[exticr_num] |= ~(ENABLE << exticr_bit_idx);
    AFIO->EXTICR[exticr_num] |= (port_num << exticr_bit_idx);

    // configure the interrupt trigger selection
    if (trigger == io_it_ft) {
        EXTI->FTSR |= (SET << exti_no);
        EXTI->RTSR &= ~(SET << exti_no);
    }
    else if (trigger == io_it_rt) {
        EXTI->RTSR |= (SET << exti_no);
        EXTI->FTSR &= ~(SET << exti_no);
    }
    else {
        EXTI->FTSR |= (SET << exti_no);
        EXTI->RTSR |= (SET << exti_no);
    }

    // unmask the interrupts on corresponding exti line
    EXTI->IMR |= (SET << exti_no);
}

static io_port_num_e retrieve_exti_port_num(IRQn_Type irq_n) {
    io_port_num_e active_exti_port = AFIO->EXTICR[irq_n >> 2] & 0xFU;
    return active_exti_port;
}

void EXTI0_IRQHandler(void) {
    // retrieve port number
    io_port_num_e port_n = retrieve_exti_port_num(exti_no_0);
    // call registered ISR
    interrupt_handlers[port_n][exti_no_0]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_no_0);
}

void EXTI1_IRQHandler(void) {
    // retrieve port number
    io_port_num_e port_n = retrieve_exti_port_num(exti_no_1);
    // call registered ISR
    interrupt_handlers[port_n][exti_no_1]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_no_1);
}

void EXTI2_IRQHandler(void) {
    // retrieve port number
    io_port_num_e port_n = retrieve_exti_port_num(exti_no_2);
    // call registered ISR
    interrupt_handlers[port_n][exti_no_2]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_no_2);
}

void EXTI3_IRQHandler(void) {
    // retrieve port number
    io_port_num_e port_n = retrieve_exti_port_num(exti_no_3);
    // call registered ISR
    interrupt_handlers[port_n][exti_no_3]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_no_3);
}

void EXTI4_IRQHandler(void) {
    // retrieve port number
    io_port_num_e port_n = retrieve_exti_port_num(exti_no_4);
    // call registered ISR
    interrupt_handlers[port_n][exti_no_4]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_no_4);
}

/**
 * Used in EXTI IRQHandlers with multiple interrupt sources. Retrieve the current EXTI line that
 * is the source of the current interrupt
 */

static inline uint8_t get_exti_line(void)
{
	for (uint8_t pin_trigger = EXTI_PR_PR5_Pos; pin_trigger < EXTI_PR_PR16_Pos; ++pin_trigger)
	{
		uint8_t check_pr_set = (EXTI->PR >> pin_trigger) & 0x1;
		if (check_pr_set == SET)
		{
			return pin_trigger;
		}
	}
	return RESET;
}

void EXTI9_5_IRQHandler(void) {
    uint8_t exti_line_no = get_exti_line();
    io_port_num_e port_n = retrieve_exti_port_num(exti_line_no);
    // call registered ISR
    interrupt_handlers[port_n][exti_line_no]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_line_no);
}

void EXTI15_10_IRQHandler(void) {
    uint8_t exti_line_no = get_exti_line();
    io_port_num_e port_n = retrieve_exti_port_num(exti_line_no);
    // call registered ISR
    interrupt_handlers[port_n][exti_line_no]();
    // clear pending bit
    EXTI->PR |= (ENABLE << exti_line_no);
}