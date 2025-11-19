#ifndef _IO_H
#define _IO_H

#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_ll_rcc.h"
#include "../../Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/**********************************
*          USER IO ENUMS
***********************************/
typedef enum
{
    IO_PIN_0,
    IO_PIN_1,
    IO_PIN_2,
    IO_PIN_3,
    IO_PIN_4,
    IO_PIN_5,
    IO_PIN_6,
    IO_PIN_7,
    IO_PIN_8,
    IO_PIN_9,
    IO_PIN_10,
    IO_PIN_11,
    IO_PIN_12,
    IO_PIN_13,
    IO_PIN_14,
    IO_PIN_15
} io_pin_no_e;

typedef enum
{
    // PORT A
    IO_A0, IO_A1, IO_A2, IO_A3,
    IO_A4, IO_A5, IO_A6, IO_A7,
    IO_A8, IO_A9, IO_A10, IO_A11,
    IO_A12, IO_A13, IO_A14, IO_A15,

    // PORT B
    IO_B0, IO_B1, IO_B2, IO_B3,
    IO_B4, IO_B5, IO_B6, IO_B7,
    IO_B8, IO_B9, IO_B10, IO_B11,
    IO_B12, IO_B13, IO_B14, IO_B15,

    // PORT C
    IO_C0, IO_C1, IO_C2, IO_C3,
    IO_C4, IO_C5, IO_C6, IO_C7,
    IO_C8, IO_C9, IO_C10, IO_C11,
    IO_C12, IO_C13, IO_C14, IO_C15
} generic_io_e;

typedef enum
{
    IO_UNUSED_0 = IO_A0,
    IO_UNUSED_1 = IO_A1,
    IO_M1_PWM = IO_A2,
    IO_M2_PWM = IO_A3,
    IO_UNUSED_4 = IO_A4,
    IO_UNUSED_5 = IO_A5,
    M1_ENC_A = IO_A6,
    M1_ENC_B = IO_A7,
    M1_DRIVER_IN1 = IO_A8,
    IO_UNUSED_9 = IO_A9,
    IO_UNUSED_10 = IO_A10,
    IO_UNUSED_11 = IO_A11,
    IO_UNUSED_12 = IO_A12,
    IO_UNUSED_13 = IO_A13,
    IO_UNUSED_14 = IO_A14,
    IO_UNUSED_15 = IO_A15,
    IO_UNUSED_16 = IO_B0,
    IO_UNUSED_17 = IO_B1,
    IO_UNUSED_18 = IO_B2,
    IO_UNUSED_19 = IO_B3,
    IO_UNUSED_20 = IO_B4,
    IO_UNUSED_21 = IO_B5,
    M2_ENC_B = IO_B6,
    M2_ENC_A = IO_B7,
    IO_UNUSED_24 = IO_B8,
    IO_UNUSED_25 = IO_B9,
    IO_UNUSED_26 = IO_B10,
    IO_UNUSED_27 = IO_B11,
    IO_UNUSED_28 = IO_B12,
    M2_DRIVER_IN1 = IO_B13,
    M1_DRIVER_IN2 = IO_B14,
    M2_DRIVER_IN2 = IO_B15,
    IO_UNUSED_32 = IO_C0,
    IO_UNUSED_33 = IO_C1,
    IO_UNUSED_34 = IO_C2,
    IO_UNUSED_35 = IO_C3,
    IO_UNUSED_36 = IO_C4,
    IO_UNUSED_37 = IO_C5,
    IO_UNUSED_38 = IO_C6,
    IO_UNUSED_39 = IO_C7,
    IO_UNUSED_40 = IO_C8,
    IO_UNUSED_41 = IO_C9,
    IO_UNUSED_42 = IO_C10,
    IO_UNUSED_43 = IO_C11,
    IO_UNUSED_44 = IO_C12,
    IO_UNUSED_45 = IO_C13,
    IO_UNUSED_46 = IO_C14,
    IO_UNUSED_47 = IO_C15
} io_e;

typedef enum
{
    io_mode_input,
    io_mode_output_10MHz,
    io_mode_output_2MHz,
    io_mode_output_50MHz
} io_mode_e;

typedef enum
{
    io_cnf_input_analog,
    io_cnf_input_float,
    io_cnf_input_pupd
} io_cnf_input_e;

typedef enum
{
    io_cnf_output_gpio_pp,
    io_cnf_output_gpio_od,
    io_cnf_output_af_pp,
    io_cnf_output_af_od
} io_cnf_output_e;

typedef enum
{
    exit_no_0,
    exti_no_1,
    exti_no_2,
    exit_no_3,
    exti_no_4,
    exti_no_5,
    exit_no_6,
    exti_no_7,
    exti_no_8,
    exit_no_9,
    exti_no_10,
    exti_no_11,
    exit_no_12,
    exti_no_13,
    exti_no_14,
    exti_no_15
} exti_no_e;

typedef enum
{
    io_it_ft,
    io_it_rt,
    io_it_ft_rt
} io_it_trigger_e;

typedef enum 
{
    LOW,
    HIGH
} io_out_e;

/**********************************
*          USER IO Structs
***********************************/
typedef struct
{
    uint8_t mode;           /* Specifies Mode: Input (default) OR Output [10/2/50MHz] */
    uint8_t mode_config;    /* Specifies Mode Config: Input (analog, floating input, push-pull) OR Output (GPIO PP/OD : AF PP/OD) */

} io_config_t;

/**********************************
*          USER IO APIs
***********************************/

/**
*   io_configure - Sets the configuration of all the pins used/unused
*   
*
*   @return void
**/
void io_configure(void);

/**
*   io_init - inits a singles io pin with the specified configuration struct
*   
*   @param io_config - io pin configuration struct, specifies mode/speed, resistance, etc.
*   @return void
**/
void io_init(io_e io, const io_config_t *io_config);

/**
 *  io_get_configuration -  retrieves the actual configuration of the initialized
 *                          pin and stores the configuration into a struct
 * 
 *  @param io - io pin to retrieve configuration of
 *  @return io_config_t
 */
void io_get_configuration(io_e io, io_config_t *actual);

/**
 *  io_verify_config -  compares configured io pin with config struct
 * 
 *  @param curr_cnf - io pin to check configuration of
 *  @param expected_cnf - expected io configuration
 *  @return void
 */
inline bool io_verify_config(io_config_t *curr_cnf, io_config_t *expected_cnf);

void io_set_out(io_e, io_out_e);

#endif