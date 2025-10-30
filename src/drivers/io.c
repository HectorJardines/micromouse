#include "../../inc/drivers/io.h"

#define PIN_COUNT           (48U)
#define PORT_COUNT          (3U)
#define IO_CRx_CNF_OFF      (2U)
#define NO_OF_HI_BITS       (8U)

#define IO_PORT_OFFSET      (4U)
#define IO_PORT_MASK        (0x3U << IO_PORT_OFFSET)
#define IO_PIN_MASK         (0x0F)

#define UNUSED_CONFIG       {io_mode_input, io_cnf_input_analog}

io_config_t pin_configurations[PIN_COUNT] = {
    // PORT A
    [IO_UNUSED_0] = UNUSED_CONFIG, [IO_UNUSED_1] = UNUSED_CONFIG, [IO_UNUSED_2] = UNUSED_CONFIG, [IO_UNUSED_3] = UNUSED_CONFIG,
    [IO_UNUSED_4] = UNUSED_CONFIG, [IO_UNUSED_5] = UNUSED_CONFIG, [IO_UNUSED_6] = UNUSED_CONFIG, [IO_UNUSED_7] = UNUSED_CONFIG,
    [IO_UNUSED_8] = UNUSED_CONFIG, [IO_UNUSED_9] = UNUSED_CONFIG, [IO_UNUSED_10] = UNUSED_CONFIG, [IO_UNUSED_11] = UNUSED_CONFIG,
    [IO_UNUSED_12] = UNUSED_CONFIG, [IO_UNUSED_13] = UNUSED_CONFIG, [IO_UNUSED_14] = UNUSED_CONFIG, [IO_UNUSED_15] = UNUSED_CONFIG,

    // PORT B
    [IO_UNUSED_16] = UNUSED_CONFIG, [IO_UNUSED_17] = UNUSED_CONFIG, [IO_UNUSED_18] = UNUSED_CONFIG, [IO_UNUSED_19] = UNUSED_CONFIG,
    [IO_UNUSED_20] = UNUSED_CONFIG, [IO_UNUSED_21] = UNUSED_CONFIG, [IO_UNUSED_22] = UNUSED_CONFIG, [IO_UNUSED_23] = UNUSED_CONFIG,
    [IO_UNUSED_24] = UNUSED_CONFIG, [IO_UNUSED_25] = UNUSED_CONFIG, [IO_UNUSED_26] = UNUSED_CONFIG, [IO_UNUSED_27] = UNUSED_CONFIG,
    [IO_UNUSED_28] = UNUSED_CONFIG, [IO_UNUSED_29] = UNUSED_CONFIG, [IO_UNUSED_30] = UNUSED_CONFIG, [IO_UNUSED_31] = UNUSED_CONFIG,

    // PORT B
    [IO_UNUSED_32] = UNUSED_CONFIG, [IO_UNUSED_33] = UNUSED_CONFIG, [IO_UNUSED_34] = UNUSED_CONFIG, [IO_UNUSED_35] = UNUSED_CONFIG,
    [IO_UNUSED_36] = UNUSED_CONFIG, [IO_UNUSED_37] = UNUSED_CONFIG, [IO_UNUSED_38] = UNUSED_CONFIG, [IO_UNUSED_39] = UNUSED_CONFIG,
    [IO_UNUSED_40] = UNUSED_CONFIG, [IO_UNUSED_41] = UNUSED_CONFIG, [IO_UNUSED_42] = UNUSED_CONFIG, [IO_UNUSED_43] = UNUSED_CONFIG,
    [IO_UNUSED_44] = UNUSED_CONFIG, [IO_UNUSED_45] = UNUSED_CONFIG, [IO_UNUSED_46] = UNUSED_CONFIG, [IO_UNUSED_47] = UNUSED_CONFIG
};

static GPIO_TypeDef *gpiox[PORT_COUNT] = { GPIOA, GPIOB, GPIOC };

/************************
 *      IO CONFIG APIs
 ************************/
/**
 * these functions allow use to retrieve the io port, pin number, and pin bit position
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

inline bool io_verify_config(io_config_t *curr_cnf, io_config_t *expected_cnf)
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
        actual->mode = (gpiox[port]->CRH & (0x3U << pin_mode_bit));
        // get pin configuration
        actual->mode_config = (gpiox[port]->CRH & (0x3U << (pin_mode_bit + IO_CRx_CNF_OFF)));
    }
    else {
        // get pin mode
        actual->mode = (gpiox[port]->CRL & (0x3U << pin_mode_bit));
        // get pin configuration
        actual->mode_config = (gpiox[port]->CRL & (0x3U << (pin_mode_bit + IO_CRx_CNF_OFF)));
    }

    // TODO: Retrieve AF configuration if present
}

void io_init(io_e io, const io_config_t *io_config)
{
    uint8_t port = io_port(io);
    uint8_t pin_idx = io_pin_idx(io);
    uint8_t pin_mode_bit = pin_idx * 4;
    
    if (pin_idx > IO_PIN_7) {
        pin_idx = pin_idx % NO_OF_HI_BITS;
        pin_mode_bit = pin_idx * 4;
        // set pin mode
        gpiox[port]->CRH |= (io_config->mode << (pin_mode_bit));
        // set pin configuration
        gpiox[port]->CRH |= (io_config->mode_config << (pin_mode_bit + IO_CRx_CNF_OFF));
    }
    else {
        // set pin mode
        gpiox[port]->CRL |= (io_config->mode << (pin_mode_bit));
        // set pin configuration
        gpiox[port]->CRL |= (io_config->mode_config << (pin_mode_bit + IO_CRx_CNF_OFF));
    }

    // TODO: AF FUNCTIONALITY

}

void io_configure(void)
{
    io_config_t actual_config;
    for (io_e pin_no = 0; pin_no < PIN_COUNT; ++pin_no) {
        io_init(pin_no, &pin_configurations[pin_no]);
        io_get_configuration(pin_no, &actual_config);
        if (io_verify_config(&actual_config, &pin_configurations[pin_no]) == false) {
            while(1) {
                // code to toggle LED
            }
        }
    }
}
