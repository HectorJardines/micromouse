#ifndef _L293D_H
#define _L293D_H
#include "pwm.h"

typedef enum {
    dir_stop,
    dir_forward,
    dir_reverse
} l293d_dir_e;

typedef enum {
    l293d_motor_left,
    l293d_motor_right
} l293d_motor_e;

void l293d_init(void);
void l293d_set_dir(pwm_channel_e pwm, l293d_dir_e dir);
void l293d_set_speed(pwm_channel_e pwm, int16_t duty_cycle);

#endif