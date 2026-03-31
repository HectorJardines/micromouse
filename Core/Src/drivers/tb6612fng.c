#include "../inc/drivers/tb6612fng.h"
#include <stdbool.h>

struct cntl_pins {
    io_e driver_in_1;
    io_e driver_in_2;
};

struct cntl_pins driver_input_cnfgs[] = {
    [tb6612fng_motor_left] = {M1_DRIVER_IN1, M1_DRIVER_IN2},
    [tb6612fng_motor_right] = {M2_DRIVER_IN1, M2_DRIVER_IN2}
};

static uint8_t limit_pwm_value(int16_t duty_cycle)
{
    return 0;
}

static bool initialized = false;
void tb6612fng_init(void)
{
    if (initialized)
        return;
    pwm_init();
    initialized = true;
}

void tb6612fng_set_dir(pwm_channel_e pwm, tb6612fng_dir_e dir)
{
    switch (dir) {
        case dir_stop:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, LOW);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, LOW);
            break;
        case dir_forward:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, HIGH);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, LOW);
            break;
        case dir_reverse:
            io_set_out(driver_input_cnfgs[pwm].driver_in_1, LOW);
            io_set_out(driver_input_cnfgs[pwm].driver_in_2, HIGH);
            break;
    }
}


void tb6612fng_set_speed(pwm_channel_e pwm, int16_t duty_cycle) {
    uint8_t duty = limit_pwm_value(duty_cycle);
    pwm_set_duty_cycle(pwm, duty);
}
