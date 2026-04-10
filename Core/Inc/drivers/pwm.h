#ifndef _PWM_H
#define _PWM_H

#include "../inc/drivers/io.h"

typedef enum {
    PWM_TB6612FNG_LEFT,
    PWM_TB6612FNG_RIGHT
} pwm_channel_e;

/**
 * @brief Initialize the TIMER2 peripheral in PWM upcounting mode
 */
void pwm_init(void);

/**
 * @param Sets the duty cycle for the specified pwm channel
 * 
 * The API expects a duty cycle value in the range [0-99], if the specified
 * duty cycle is >0 the timer peripheral will be enabled. If both channels have 
 * a pwm duty cycle value =0 the peripheral will be disabled.
 * 
 * @param pwm The pwm timer peripheral channel for which PWM duty cycle will be set
 * @param duty Duty cyle value
 */
void pwm_set_duty_cycle(pwm_channel_e pwm, int32_t duty);

#endif