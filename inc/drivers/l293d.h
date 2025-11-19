#ifndef _L293D_H
#define _L293D_H
#include "pwm.h"

typedef enum {
    dir_stop,
    dir_forward,
    dir_reverse
    // dir_turn_right,
    // dir_turn_left,
    // dir_rotate_right,
    // dir_rotate_left
} l293d_dir_e;

typedef enum {
    l293d_motor_left,
    l293d_motor_right
} l293d_motor_e;

void l293d_init(void);
void l293d_set_drive(pwm_channel_e pwm, l293d_dir_e dir, uint16_t duty_cycle_percentage);

#endif