#include "../inc/common/assert_handler.h"
#include "../inc/drivers/io.h"
#include "../inc/drivers/uart.h"
#include "../inc/main.h"
#include "../external/printf/printf.h"
#include "../Inc/drivers/encoder.h"
#include "../Inc/common/log.h"
#include "../Inc/drivers/pid.h"
#include "../Inc/drivers/pwm.h"

void test_setup(void) {
    SystemClock_Config();
    // HAL_MspInit();
    io_configure();
    log_init();
}

void test_uart(void) {
    usart_init();
    char msg[7] = "Hello\n";
    while(1) {
        usart_write(msg, 6);
    }
}

void test_putchar(void) {
    usart_init();

    while(1) {
        _putchar('H');
        _putchar('e');
        _putchar('l');
        _putchar('l');
        _putchar('o');
        _putchar('\n');
    }
}

void test_assert(void) {
    ASSERT(0, ASSERT_DRIVER_LEVEL);
}

void test_log(void) {
    // log_init();

    while(1) {
        LOG("HELLO IM GAY...\n");
    }
}

void test_encoder(void) {
    // log_init();
    encoder_init();

    uint16_t left_count = 0;
    uint16_t right_count = 0;

    while (1) {
        left_count = encoder_read_left_count();
        right_count = encoder_read_right_count();

        LOG("ENCODER COUNT RIGHT = %d\n", right_count);
        LOG("ENCODER COUNT LEFT = %d\n", left_count);

        HAL_Delay(500);
    }
}

void test_pwm(void) {
    pwm_init();
    // io_set_out(IO_UNUSED_24, HIGH);
    io_set_out(M1_DRIVER_IN1, LOW);
    io_set_out(M1_DRIVER_IN2, LOW);

    io_set_out(M2_DRIVER_IN1, LOW);
    io_set_out(M2_DRIVER_IN2, LOW);

    while (1) {
        pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, 30);
        pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT, 50);
        HAL_Delay(100);

        pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, 45);
        pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT, 45);
        HAL_Delay(100);

        pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, 60);
        pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT, 60);
        HAL_Delay(100);

        pwm_set_duty_cycle(PWM_TB6612FNG_LEFT, 75);
        pwm_set_duty_cycle(PWM_TB6612FNG_RIGHT, 75);
        HAL_Delay(100);
    }
}

void test_pid(void) {
    pid_init();
    
    // set GOAL to 18cm away
    set_pid_goal_dist(PID_DIST_TO_ENC_COUNT(18));
    while(1) {
        pid_update();
        // HAL_Delay(1);
        if (pid_done()) {
            // reset_pid();
            // set_pid_goal_dist(PID_DIST_TO_ENC_COUNT(18));
            LOG("TRAVELED 18 CM...\n");

            break;
        }
    }

    while(1) {
        
        HAL_Delay(100);
        io_toggle_out(IO_LED_RED);
        HAL_Delay(100);
        io_toggle_out(IO_LED_BLUE);
        HAL_Delay(100);
        io_toggle_out(IO_LED_GREEN);

    }
}

int main(void) {
    test_setup();
    test_pid();
}