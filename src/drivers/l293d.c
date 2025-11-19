#include "../inc/drivers/l293d.h"
#include <stdbool.h>

struct cntl_pins {
    io_e driver_in_1;
    io_e driver_in_2;
};

struct cntl_pins driver_input_cnfgs[] = {
    [l293d_motor_left] = {M1_DRIVER_IN1, M1_DRIVER_IN2},
    [l293d_motor_right] = {M2_DRIVER_IN1, M2_DRIVER_IN2}
};

static bool initialized = false;
void l293d_init(void)
{
    if (initialized)
        return;
    pwm_init();
    initialized = true;
}

void l293d_set_dir(pwm_channel_e pwm, l293d_dir_e dir)
{
    switch (dir) {
        case dir_stop:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, LOW);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, LOW);
            break;
        case dir_forward:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, LOW);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, HIGH);
            break;
        case dir_reverse:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, HIGH);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, LOW);
            break;
    }
}

void l293d_set_speed(pwm_channel_e pwm, uint8_t duty_cycle) {
    // pwm_channel_enable(pwm, ENABLE);
    pwm_set_duty_cycle(pwm, duty_cycle);
}
