#ifndef _TB6612FNG_H
#define _TB6612FNG_H
#include "../inc/drivers/pwm.h"

typedef enum {
    dir_stop,
    dir_forward,
    dir_reverse
} tb6612fng_dir_e;

typedef enum {
    tb6612fng_motor_left,
    tb6612fng_motor_right
} tb6612fng_motor_e;

void tb6612fng_init(void);
void tb6612fng_set_dir(pwm_channel_e pwm, tb6612fng_dir_e dir);
void tb6612fng_set_speed(pwm_channel_e pwm, int16_t duty_cycle);

#endif