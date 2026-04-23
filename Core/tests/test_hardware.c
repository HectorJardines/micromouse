#include "../inc/common/assert_handler.h"
#include "../inc/drivers/io.h"
#include "../inc/drivers/uart.h"
#include "../inc/main.h"
#include "../external/printf/printf.h"
#include "../Inc/drivers/encoder.h"
#include "../Inc/common/log.h"
#include "../Inc/drivers/pid.h"
#include "../Inc/drivers/pwm.h"
#include "../Inc/drivers/imu_interface.h"
#include "../Inc/drivers/ssd1306.h"
#include "../Inc/drivers/ir_interface.h"

void test_setup(void) {
    SystemClock_Config();
    // HAL_MspInit();
    io_configure();
    log_init();
    log_set_level(LOG_LEVEL_DEBUG);
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
        LOG(LOG_LEVEL_DEBUG, "HELLO IM GAY...\n");
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

        LOG(LOG_LEVEL_DEBUG, "ENCODER COUNT RIGHT = %d\n", right_count);
        LOG(LOG_LEVEL_DEBUG, "ENCODER COUNT LEFT = %d\n", left_count);

        HAL_Delay(500);
    }
}

void test_pwm(void) {
    pwm_init();
    // io_set_out(IO_UNUSED_24, HIGH);
    io_set_out(M1_DRIVER_IN1, HIGH);
    io_set_out(M1_DRIVER_IN2, LOW);

    io_set_out(M2_DRIVER_IN1, HIGH);
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
            LOG(LOG_LEVEL_DEBUG, "TRAVELED 18 CM...\n");

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

void test_imu_self_test(void) {
    imu_init();

    while(1) {
        int8_t res = imu_self_test_accel();
        // if (!res)
            // LOG("IMU ACCEL SELF TEST PASS...\n");
        res = imu_self_test_gyro();
        // if (!res)
            // LOG("IMU GYRO SELF TEST PASS...\n");
        
        if (res) {
            while(1) {
                HAL_Delay(100);
                io_toggle_out(IO_LED_RED);
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
}

void test_imu_read_accel_values(void) {
    imu_init();

    struct bmi160_sensor_data accel;
    struct bmi160_sensor_data gyro;

    while (1) {
        int8_t res = imu_read_sensor_data(&accel, &gyro);
        if (res) {
            LOG(LOG_LEVEL_DEBUG, "ERRO RETRIEVING DATA...\n");
            while(1) {
                HAL_Delay(100);
                io_toggle_out(IO_LED_RED);
            } 
        }

        LOG(LOG_LEVEL_DEBUG, "ACCEL - X: %d, Y: %d, Z: %d\n", accel.x, accel.y, accel.z);
        LOG(LOG_LEVEL_DEBUG, "GYRO - X: %d, Y: %d, Z: %d\n", gyro.x, gyro.y, gyro.z);
        HAL_Delay(500);
    }
}

void test_imu_read_left_right_drift(void) {
    imu_init();
    float angle_z = 0;

    while (1) {
        
        int8_t res = imu_get_angle_z(&angle_z, .01);
        if (res) {
            LOG(LOG_LEVEL_DEBUG, "ERRO RETRIEVING PITCH/ROLL...\n");
            while(1) {
                HAL_Delay(100);
                io_toggle_out(IO_LED_RED);
            } 
        }

        LOG(LOG_LEVEL_DEBUG, "ANGLE Z: %f degrees\n", angle_z);

        HAL_Delay(10);
    }
}

void test_pid_with_angle_correction(void) {
    pid_init();

    set_pid_goal_dist(PID_DIST_TO_ENC_COUNT(100));
    // set_pid_goal_angle(PID_ANGLE_TO_ENC_COUNT(0));
    while(1) {
        pid_update();
        // HAL_Delay(1);
        if (pid_done()) {
            // reset_pid();
            // set_pid_goal_dist(PID_DIST_TO_ENC_COUNT(18));
            LOG(LOG_LEVEL_DEBUG, "TRAVELED 100 CM...\n");

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

void test_oled_control(void) {
    ssd1306_init();
    //id: 0 pixel 3 
    ssd1306_draw_pixel(14, 9, COLOR_WHITE);
    ssd1306_draw_pixel(14, 10, COLOR_WHITE);
    ssd1306_draw_pixel(14, 11, COLOR_WHITE);
    ssd1306_draw_pixel(14, 12, COLOR_WHITE);
    ssd1306_draw_pixel(14, 13, COLOR_WHITE);
    ssd1306_draw_pixel(14, 14, COLOR_WHITE);
    ssd1306_draw_pixel(14, 15, COLOR_WHITE);
    ssd1306_draw_pixel(14, 16, COLOR_WHITE);
    ssd1306_draw_pixel(14, 17, COLOR_WHITE);
    ssd1306_draw_pixel(14, 18, COLOR_WHITE);
    ssd1306_draw_pixel(14, 19, COLOR_WHITE);
    ssd1306_draw_pixel(14, 20, COLOR_WHITE);
    ssd1306_draw_pixel(14, 21, COLOR_WHITE);
    ssd1306_draw_pixel(14, 22, COLOR_WHITE);
    ssd1306_draw_pixel(14, 23, COLOR_WHITE);
    ssd1306_draw_pixel(14, 24, COLOR_WHITE);
    ssd1306_draw_pixel(14, 25, COLOR_WHITE);
    ssd1306_draw_pixel(14, 26, COLOR_WHITE);
    ssd1306_draw_pixel(14, 27, COLOR_WHITE);
    ssd1306_draw_pixel(14, 28, COLOR_WHITE);
    ssd1306_draw_pixel(14, 29, COLOR_WHITE);
    ssd1306_draw_pixel(14, 30, COLOR_WHITE);
    ssd1306_draw_pixel(14, 31, COLOR_WHITE);
    ssd1306_draw_pixel(14, 32, COLOR_WHITE);
    ssd1306_draw_pixel(14, 33, COLOR_WHITE);
    ssd1306_draw_pixel(14, 34, COLOR_WHITE);
    ssd1306_draw_pixel(14, 35, COLOR_WHITE);
    ssd1306_draw_pixel(14, 36, COLOR_WHITE);
    ssd1306_draw_pixel(14, 37, COLOR_WHITE);
    ssd1306_draw_pixel(14, 38, COLOR_WHITE);
    ssd1306_draw_pixel(14, 39, COLOR_WHITE);
    ssd1306_draw_pixel(14, 40, COLOR_WHITE);
    ssd1306_draw_pixel(14, 41, COLOR_WHITE);
    ssd1306_draw_pixel(14, 42, COLOR_WHITE);
    ssd1306_draw_pixel(14, 43, COLOR_WHITE);
    ssd1306_draw_pixel(14, 44, COLOR_WHITE);
    ssd1306_draw_pixel(14, 45, COLOR_WHITE);
    ssd1306_draw_pixel(14, 46, COLOR_WHITE);
    ssd1306_draw_pixel(14, 47, COLOR_WHITE);
    ssd1306_draw_pixel(14, 48, COLOR_WHITE);
    ssd1306_draw_pixel(14, 49, COLOR_WHITE);
    ssd1306_draw_pixel(14, 50, COLOR_WHITE);
    ssd1306_draw_pixel(14, 51, COLOR_WHITE);
    ssd1306_draw_pixel(14, 52, COLOR_WHITE);
    ssd1306_draw_pixel(14, 53, COLOR_WHITE);
    ssd1306_draw_pixel(14, 54, COLOR_WHITE);
    ssd1306_draw_pixel(14, 55, COLOR_WHITE);
    ssd1306_draw_pixel(14, 56, COLOR_WHITE);
    ssd1306_draw_pixel(14, 57, COLOR_WHITE);
    //id: 1 pixel 4 
    ssd1306_draw_pixel(28, 8, COLOR_WHITE);
    ssd1306_draw_pixel(27, 8, COLOR_WHITE);
    ssd1306_draw_pixel(27, 9, COLOR_WHITE);
    ssd1306_draw_pixel(27, 10, COLOR_WHITE);
    ssd1306_draw_pixel(27, 11, COLOR_WHITE);
    ssd1306_draw_pixel(27, 12, COLOR_WHITE);
    ssd1306_draw_pixel(27, 13, COLOR_WHITE);
    ssd1306_draw_pixel(27, 14, COLOR_WHITE);
    ssd1306_draw_pixel(27, 15, COLOR_WHITE);
    ssd1306_draw_pixel(27, 16, COLOR_WHITE);
    ssd1306_draw_pixel(27, 17, COLOR_WHITE);
    ssd1306_draw_pixel(27, 18, COLOR_WHITE);
    ssd1306_draw_pixel(27, 19, COLOR_WHITE);
    ssd1306_draw_pixel(27, 20, COLOR_WHITE);
    ssd1306_draw_pixel(27, 21, COLOR_WHITE);
    ssd1306_draw_pixel(27, 22, COLOR_WHITE);
    ssd1306_draw_pixel(27, 23, COLOR_WHITE);
    ssd1306_draw_pixel(27, 24, COLOR_WHITE);
    ssd1306_draw_pixel(27, 25, COLOR_WHITE);
    ssd1306_draw_pixel(27, 26, COLOR_WHITE);
    ssd1306_draw_pixel(27, 27, COLOR_WHITE);
    ssd1306_draw_pixel(27, 28, COLOR_WHITE);
    ssd1306_draw_pixel(27, 29, COLOR_WHITE);
    ssd1306_draw_pixel(27, 30, COLOR_WHITE);
    ssd1306_draw_pixel(27, 31, COLOR_WHITE);
    ssd1306_draw_pixel(27, 32, COLOR_WHITE);
    ssd1306_draw_pixel(27, 33, COLOR_WHITE);
    ssd1306_draw_pixel(27, 34, COLOR_WHITE);
    ssd1306_draw_pixel(27, 35, COLOR_WHITE);
    ssd1306_draw_pixel(27, 36, COLOR_WHITE);
    ssd1306_draw_pixel(27, 37, COLOR_WHITE);
    ssd1306_draw_pixel(27, 38, COLOR_WHITE);
    ssd1306_draw_pixel(27, 39, COLOR_WHITE);
    ssd1306_draw_pixel(27, 40, COLOR_WHITE);
    ssd1306_draw_pixel(27, 41, COLOR_WHITE);
    ssd1306_draw_pixel(27, 42, COLOR_WHITE);
    ssd1306_draw_pixel(27, 43, COLOR_WHITE);
    ssd1306_draw_pixel(27, 44, COLOR_WHITE);
    ssd1306_draw_pixel(27, 45, COLOR_WHITE);
    ssd1306_draw_pixel(27, 46, COLOR_WHITE);
    ssd1306_draw_pixel(27, 47, COLOR_WHITE);
    ssd1306_draw_pixel(27, 48, COLOR_WHITE);
    ssd1306_draw_pixel(27, 49, COLOR_WHITE);
    ssd1306_draw_pixel(27, 50, COLOR_WHITE);
    ssd1306_draw_pixel(27, 51, COLOR_WHITE);
    ssd1306_draw_pixel(27, 52, COLOR_WHITE);
    ssd1306_draw_pixel(27, 53, COLOR_WHITE);
    ssd1306_draw_pixel(27, 54, COLOR_WHITE);
    ssd1306_draw_pixel(27, 55, COLOR_WHITE);
    ssd1306_draw_pixel(28, 55, COLOR_WHITE);
    ssd1306_draw_pixel(28, 56, COLOR_WHITE);
    ssd1306_draw_pixel(28, 57, COLOR_WHITE);
    //id: 2 pixel 5 
    ssd1306_draw_pixel(14, 32, COLOR_WHITE);
    ssd1306_draw_pixel(15, 32, COLOR_WHITE);
    ssd1306_draw_pixel(16, 32, COLOR_WHITE);
    ssd1306_draw_pixel(17, 32, COLOR_WHITE);
    ssd1306_draw_pixel(18, 32, COLOR_WHITE);
    ssd1306_draw_pixel(19, 32, COLOR_WHITE);
    ssd1306_draw_pixel(20, 32, COLOR_WHITE);
    ssd1306_draw_pixel(21, 32, COLOR_WHITE);
    ssd1306_draw_pixel(22, 32, COLOR_WHITE);
    ssd1306_draw_pixel(23, 32, COLOR_WHITE);
    ssd1306_draw_pixel(24, 32, COLOR_WHITE);
    ssd1306_draw_pixel(25, 32, COLOR_WHITE);
    ssd1306_draw_pixel(26, 32, COLOR_WHITE);
    //id: 3 pixel 6 
    ssd1306_draw_pixel(35, 31, COLOR_WHITE);
    ssd1306_draw_pixel(36, 31, COLOR_WHITE);
    ssd1306_draw_pixel(37, 31, COLOR_WHITE);
    ssd1306_draw_pixel(38, 31, COLOR_WHITE);
    ssd1306_draw_pixel(39, 31, COLOR_WHITE);
    ssd1306_draw_pixel(40, 31, COLOR_WHITE);
    ssd1306_draw_pixel(41, 31, COLOR_WHITE);
    ssd1306_draw_pixel(42, 31, COLOR_WHITE);
    ssd1306_draw_pixel(43, 31, COLOR_WHITE);
    ssd1306_draw_pixel(44, 31, COLOR_WHITE);
    ssd1306_draw_pixel(44, 30, COLOR_WHITE);
    ssd1306_draw_pixel(45, 30, COLOR_WHITE);
    ssd1306_draw_pixel(46, 30, COLOR_WHITE);
    ssd1306_draw_pixel(47, 30, COLOR_WHITE);
    ssd1306_draw_pixel(48, 30, COLOR_WHITE);
    ssd1306_draw_pixel(49, 30, COLOR_WHITE);
    ssd1306_draw_pixel(50, 30, COLOR_WHITE);
    ssd1306_draw_pixel(51, 30, COLOR_WHITE);
    ssd1306_draw_pixel(51, 29, COLOR_WHITE);
    ssd1306_draw_pixel(52, 29, COLOR_WHITE);
    ssd1306_draw_pixel(52, 28, COLOR_WHITE);
    ssd1306_draw_pixel(53, 28, COLOR_WHITE);
    ssd1306_draw_pixel(53, 27, COLOR_WHITE);
    ssd1306_draw_pixel(53, 26, COLOR_WHITE);
    ssd1306_draw_pixel(54, 25, COLOR_WHITE);
    ssd1306_draw_pixel(54, 24, COLOR_WHITE);
    ssd1306_draw_pixel(54, 23, COLOR_WHITE);
    ssd1306_draw_pixel(53, 23, COLOR_WHITE);
    ssd1306_draw_pixel(53, 22, COLOR_WHITE);
    ssd1306_draw_pixel(52, 21, COLOR_WHITE);
    ssd1306_draw_pixel(52, 20, COLOR_WHITE);
    ssd1306_draw_pixel(51, 20, COLOR_WHITE);
    ssd1306_draw_pixel(51, 19, COLOR_WHITE);
    ssd1306_draw_pixel(50, 19, COLOR_WHITE);
    ssd1306_draw_pixel(49, 19, COLOR_WHITE);
    ssd1306_draw_pixel(49, 18, COLOR_WHITE);
    ssd1306_draw_pixel(48, 18, COLOR_WHITE);
    ssd1306_draw_pixel(47, 18, COLOR_WHITE);
    ssd1306_draw_pixel(46, 18, COLOR_WHITE);
    ssd1306_draw_pixel(45, 18, COLOR_WHITE);
    ssd1306_draw_pixel(44, 18, COLOR_WHITE);
    ssd1306_draw_pixel(43, 18, COLOR_WHITE);
    ssd1306_draw_pixel(42, 19, COLOR_WHITE);
    ssd1306_draw_pixel(41, 19, COLOR_WHITE);
    ssd1306_draw_pixel(40, 19, COLOR_WHITE);
    ssd1306_draw_pixel(40, 20, COLOR_WHITE);
    ssd1306_draw_pixel(39, 20, COLOR_WHITE);
    ssd1306_draw_pixel(38, 21, COLOR_WHITE);
    ssd1306_draw_pixel(37, 21, COLOR_WHITE);
    ssd1306_draw_pixel(37, 22, COLOR_WHITE);
    ssd1306_draw_pixel(36, 22, COLOR_WHITE);
    ssd1306_draw_pixel(36, 23, COLOR_WHITE);
    ssd1306_draw_pixel(36, 24, COLOR_WHITE);
    ssd1306_draw_pixel(36, 25, COLOR_WHITE);
    ssd1306_draw_pixel(36, 26, COLOR_WHITE);
    ssd1306_draw_pixel(36, 27, COLOR_WHITE);
    ssd1306_draw_pixel(35, 27, COLOR_WHITE);
    ssd1306_draw_pixel(35, 28, COLOR_WHITE);
    ssd1306_draw_pixel(35, 29, COLOR_WHITE);
    ssd1306_draw_pixel(35, 30, COLOR_WHITE);
    ssd1306_draw_pixel(35, 32, COLOR_WHITE);
    ssd1306_draw_pixel(35, 33, COLOR_WHITE);
    ssd1306_draw_pixel(35, 34, COLOR_WHITE);
    ssd1306_draw_pixel(36, 34, COLOR_WHITE);
    ssd1306_draw_pixel(36, 35, COLOR_WHITE);
    ssd1306_draw_pixel(36, 36, COLOR_WHITE);
    ssd1306_draw_pixel(36, 37, COLOR_WHITE);
    ssd1306_draw_pixel(37, 37, COLOR_WHITE);
    ssd1306_draw_pixel(37, 38, COLOR_WHITE);
    ssd1306_draw_pixel(38, 38, COLOR_WHITE);
    ssd1306_draw_pixel(38, 39, COLOR_WHITE);
    ssd1306_draw_pixel(39, 39, COLOR_WHITE);
    ssd1306_draw_pixel(39, 40, COLOR_WHITE);
    ssd1306_draw_pixel(40, 40, COLOR_WHITE);
    ssd1306_draw_pixel(41, 41, COLOR_WHITE);
    ssd1306_draw_pixel(42, 41, COLOR_WHITE);
    ssd1306_draw_pixel(42, 42, COLOR_WHITE);
    ssd1306_draw_pixel(43, 42, COLOR_WHITE);
    ssd1306_draw_pixel(44, 42, COLOR_WHITE);
    ssd1306_draw_pixel(44, 43, COLOR_WHITE);
    ssd1306_draw_pixel(45, 43, COLOR_WHITE);
    ssd1306_draw_pixel(45, 44, COLOR_WHITE);
    ssd1306_draw_pixel(46, 44, COLOR_WHITE);
    ssd1306_draw_pixel(46, 45, COLOR_WHITE);
    ssd1306_draw_pixel(47, 45, COLOR_WHITE);
    ssd1306_draw_pixel(48, 45, COLOR_WHITE);
    ssd1306_draw_pixel(49, 45, COLOR_WHITE);
    ssd1306_draw_pixel(50, 45, COLOR_WHITE);
    ssd1306_draw_pixel(51, 45, COLOR_WHITE);
    ssd1306_draw_pixel(52, 45, COLOR_WHITE);
    ssd1306_draw_pixel(53, 45, COLOR_WHITE);
    ssd1306_draw_pixel(54, 45, COLOR_WHITE);
    //id: 4 pixel 7 
    ssd1306_draw_pixel(58, 9, COLOR_WHITE);
    ssd1306_draw_pixel(58, 10, COLOR_WHITE);
    ssd1306_draw_pixel(58, 11, COLOR_WHITE);
    ssd1306_draw_pixel(58, 12, COLOR_WHITE);
    ssd1306_draw_pixel(57, 12, COLOR_WHITE);
    ssd1306_draw_pixel(57, 13, COLOR_WHITE);
    ssd1306_draw_pixel(57, 14, COLOR_WHITE);
    ssd1306_draw_pixel(57, 15, COLOR_WHITE);
    ssd1306_draw_pixel(57, 16, COLOR_WHITE);
    ssd1306_draw_pixel(57, 17, COLOR_WHITE);
    ssd1306_draw_pixel(57, 18, COLOR_WHITE);
    ssd1306_draw_pixel(57, 19, COLOR_WHITE);
    ssd1306_draw_pixel(57, 20, COLOR_WHITE);
    ssd1306_draw_pixel(57, 21, COLOR_WHITE);
    ssd1306_draw_pixel(57, 22, COLOR_WHITE);
    ssd1306_draw_pixel(58, 23, COLOR_WHITE);
    ssd1306_draw_pixel(58, 24, COLOR_WHITE);
    ssd1306_draw_pixel(58, 25, COLOR_WHITE);
    ssd1306_draw_pixel(58, 26, COLOR_WHITE);
    ssd1306_draw_pixel(58, 27, COLOR_WHITE);
    ssd1306_draw_pixel(58, 28, COLOR_WHITE);
    ssd1306_draw_pixel(58, 29, COLOR_WHITE);
    ssd1306_draw_pixel(58, 30, COLOR_WHITE);
    ssd1306_draw_pixel(58, 31, COLOR_WHITE);
    ssd1306_draw_pixel(58, 32, COLOR_WHITE);
    ssd1306_draw_pixel(58, 33, COLOR_WHITE);
    ssd1306_draw_pixel(58, 34, COLOR_WHITE);
    ssd1306_draw_pixel(58, 35, COLOR_WHITE);
    ssd1306_draw_pixel(58, 36, COLOR_WHITE);
    ssd1306_draw_pixel(58, 37, COLOR_WHITE);
    ssd1306_draw_pixel(58, 38, COLOR_WHITE);
    ssd1306_draw_pixel(58, 39, COLOR_WHITE);
    ssd1306_draw_pixel(58, 40, COLOR_WHITE);
    ssd1306_draw_pixel(58, 41, COLOR_WHITE);
    ssd1306_draw_pixel(58, 42, COLOR_WHITE);
    ssd1306_draw_pixel(58, 43, COLOR_WHITE);
    ssd1306_draw_pixel(58, 44, COLOR_WHITE);
    ssd1306_draw_pixel(58, 45, COLOR_WHITE);
    ssd1306_draw_pixel(58, 46, COLOR_WHITE);
    ssd1306_draw_pixel(58, 47, COLOR_WHITE);
    ssd1306_draw_pixel(58, 48, COLOR_WHITE);
    ssd1306_draw_pixel(58, 49, COLOR_WHITE);
    ssd1306_draw_pixel(58, 50, COLOR_WHITE);
    //id: 5 pixel 8 
    ssd1306_draw_pixel(66, 8, COLOR_WHITE);
    ssd1306_draw_pixel(66, 9, COLOR_WHITE);
    ssd1306_draw_pixel(66, 10, COLOR_WHITE);
    ssd1306_draw_pixel(66, 11, COLOR_WHITE);
    ssd1306_draw_pixel(67, 11, COLOR_WHITE);
    ssd1306_draw_pixel(67, 12, COLOR_WHITE);
    ssd1306_draw_pixel(67, 13, COLOR_WHITE);
    ssd1306_draw_pixel(67, 14, COLOR_WHITE);
    ssd1306_draw_pixel(67, 15, COLOR_WHITE);
    ssd1306_draw_pixel(67, 16, COLOR_WHITE);
    ssd1306_draw_pixel(67, 17, COLOR_WHITE);
    ssd1306_draw_pixel(67, 18, COLOR_WHITE);
    ssd1306_draw_pixel(67, 19, COLOR_WHITE);
    ssd1306_draw_pixel(67, 20, COLOR_WHITE);
    ssd1306_draw_pixel(67, 21, COLOR_WHITE);
    ssd1306_draw_pixel(67, 22, COLOR_WHITE);
    ssd1306_draw_pixel(67, 23, COLOR_WHITE);
    ssd1306_draw_pixel(67, 24, COLOR_WHITE);
    ssd1306_draw_pixel(67, 25, COLOR_WHITE);
    ssd1306_draw_pixel(67, 26, COLOR_WHITE);
    ssd1306_draw_pixel(67, 27, COLOR_WHITE);
    ssd1306_draw_pixel(68, 27, COLOR_WHITE);
    ssd1306_draw_pixel(68, 28, COLOR_WHITE);
    ssd1306_draw_pixel(68, 29, COLOR_WHITE);
    ssd1306_draw_pixel(68, 30, COLOR_WHITE);
    ssd1306_draw_pixel(68, 31, COLOR_WHITE);
    ssd1306_draw_pixel(68, 32, COLOR_WHITE);
    ssd1306_draw_pixel(68, 33, COLOR_WHITE);
    ssd1306_draw_pixel(68, 34, COLOR_WHITE);
    ssd1306_draw_pixel(68, 35, COLOR_WHITE);
    ssd1306_draw_pixel(67, 35, COLOR_WHITE);
    ssd1306_draw_pixel(67, 36, COLOR_WHITE);
    ssd1306_draw_pixel(67, 37, COLOR_WHITE);
    ssd1306_draw_pixel(67, 38, COLOR_WHITE);
    ssd1306_draw_pixel(67, 39, COLOR_WHITE);
    ssd1306_draw_pixel(67, 40, COLOR_WHITE);
    ssd1306_draw_pixel(67, 41, COLOR_WHITE);
    ssd1306_draw_pixel(67, 42, COLOR_WHITE);
    ssd1306_draw_pixel(67, 43, COLOR_WHITE);
    ssd1306_draw_pixel(67, 44, COLOR_WHITE);
    ssd1306_draw_pixel(67, 45, COLOR_WHITE);
    ssd1306_draw_pixel(67, 46, COLOR_WHITE);
    ssd1306_draw_pixel(67, 47, COLOR_WHITE);
    ssd1306_draw_pixel(67, 48, COLOR_WHITE);
    ssd1306_draw_pixel(67, 49, COLOR_WHITE);
    ssd1306_draw_pixel(67, 50, COLOR_WHITE);
    ssd1306_draw_pixel(67, 51, COLOR_WHITE);
    ssd1306_draw_pixel(67, 52, COLOR_WHITE);
    //id: 6 pixel 9 
    ssd1306_draw_pixel(91, 15, COLOR_WHITE);
    ssd1306_draw_pixel(90, 15, COLOR_WHITE);
    ssd1306_draw_pixel(89, 15, COLOR_WHITE);
    ssd1306_draw_pixel(88, 15, COLOR_WHITE);
    ssd1306_draw_pixel(87, 15, COLOR_WHITE);
    ssd1306_draw_pixel(87, 16, COLOR_WHITE);
    ssd1306_draw_pixel(86, 16, COLOR_WHITE);
    ssd1306_draw_pixel(85, 16, COLOR_WHITE);
    ssd1306_draw_pixel(84, 17, COLOR_WHITE);
    ssd1306_draw_pixel(83, 17, COLOR_WHITE);
    ssd1306_draw_pixel(82, 17, COLOR_WHITE);
    ssd1306_draw_pixel(82, 18, COLOR_WHITE);
    ssd1306_draw_pixel(81, 18, COLOR_WHITE);
    ssd1306_draw_pixel(81, 19, COLOR_WHITE);
    ssd1306_draw_pixel(80, 19, COLOR_WHITE);
    ssd1306_draw_pixel(80, 20, COLOR_WHITE);
    ssd1306_draw_pixel(79, 20, COLOR_WHITE);
    ssd1306_draw_pixel(79, 21, COLOR_WHITE);
    ssd1306_draw_pixel(79, 22, COLOR_WHITE);
    ssd1306_draw_pixel(78, 22, COLOR_WHITE);
    ssd1306_draw_pixel(78, 23, COLOR_WHITE);
    ssd1306_draw_pixel(78, 24, COLOR_WHITE);
    ssd1306_draw_pixel(77, 25, COLOR_WHITE);
    ssd1306_draw_pixel(77, 26, COLOR_WHITE);
    ssd1306_draw_pixel(77, 27, COLOR_WHITE);
    ssd1306_draw_pixel(77, 28, COLOR_WHITE);
    ssd1306_draw_pixel(77, 29, COLOR_WHITE);
    ssd1306_draw_pixel(76, 29, COLOR_WHITE);
    ssd1306_draw_pixel(76, 30, COLOR_WHITE);
    ssd1306_draw_pixel(76, 31, COLOR_WHITE);
    ssd1306_draw_pixel(76, 32, COLOR_WHITE);
    ssd1306_draw_pixel(76, 33, COLOR_WHITE);
    ssd1306_draw_pixel(76, 34, COLOR_WHITE);
    ssd1306_draw_pixel(76, 35, COLOR_WHITE);
    ssd1306_draw_pixel(76, 36, COLOR_WHITE);
    ssd1306_draw_pixel(76, 37, COLOR_WHITE);
    ssd1306_draw_pixel(76, 38, COLOR_WHITE);
    ssd1306_draw_pixel(76, 39, COLOR_WHITE);
    ssd1306_draw_pixel(76, 40, COLOR_WHITE);
    ssd1306_draw_pixel(76, 41, COLOR_WHITE);
    ssd1306_draw_pixel(76, 42, COLOR_WHITE);
    ssd1306_draw_pixel(76, 43, COLOR_WHITE);
    ssd1306_draw_pixel(76, 44, COLOR_WHITE);
    ssd1306_draw_pixel(77, 44, COLOR_WHITE);
    ssd1306_draw_pixel(77, 45, COLOR_WHITE);
    ssd1306_draw_pixel(77, 46, COLOR_WHITE);
    ssd1306_draw_pixel(78, 46, COLOR_WHITE);
    ssd1306_draw_pixel(78, 47, COLOR_WHITE);
    ssd1306_draw_pixel(79, 47, COLOR_WHITE);
    ssd1306_draw_pixel(79, 48, COLOR_WHITE);
    ssd1306_draw_pixel(80, 48, COLOR_WHITE);
    ssd1306_draw_pixel(80, 49, COLOR_WHITE);
    ssd1306_draw_pixel(81, 49, COLOR_WHITE);
    ssd1306_draw_pixel(82, 49, COLOR_WHITE);
    ssd1306_draw_pixel(82, 50, COLOR_WHITE);
    ssd1306_draw_pixel(83, 50, COLOR_WHITE);
    ssd1306_draw_pixel(84, 50, COLOR_WHITE);
    ssd1306_draw_pixel(85, 51, COLOR_WHITE);
    ssd1306_draw_pixel(86, 51, COLOR_WHITE);
    ssd1306_draw_pixel(87, 51, COLOR_WHITE);
    ssd1306_draw_pixel(88, 51, COLOR_WHITE);
    ssd1306_draw_pixel(89, 51, COLOR_WHITE);
    ssd1306_draw_pixel(90, 51, COLOR_WHITE);
    ssd1306_draw_pixel(91, 51, COLOR_WHITE);
    ssd1306_draw_pixel(92, 50, COLOR_WHITE);
    ssd1306_draw_pixel(93, 50, COLOR_WHITE);
    ssd1306_draw_pixel(93, 49, COLOR_WHITE);
    ssd1306_draw_pixel(94, 49, COLOR_WHITE);
    ssd1306_draw_pixel(94, 48, COLOR_WHITE);
    ssd1306_draw_pixel(95, 48, COLOR_WHITE);
    ssd1306_draw_pixel(95, 47, COLOR_WHITE);
    ssd1306_draw_pixel(96, 47, COLOR_WHITE);
    ssd1306_draw_pixel(96, 46, COLOR_WHITE);
    ssd1306_draw_pixel(97, 46, COLOR_WHITE);
    ssd1306_draw_pixel(97, 45, COLOR_WHITE);
    ssd1306_draw_pixel(98, 44, COLOR_WHITE);
    ssd1306_draw_pixel(98, 43, COLOR_WHITE);
    ssd1306_draw_pixel(98, 42, COLOR_WHITE);
    ssd1306_draw_pixel(99, 42, COLOR_WHITE);
    ssd1306_draw_pixel(99, 41, COLOR_WHITE);
    ssd1306_draw_pixel(99, 40, COLOR_WHITE);
    ssd1306_draw_pixel(100, 40, COLOR_WHITE);
    ssd1306_draw_pixel(100, 39, COLOR_WHITE);
    ssd1306_draw_pixel(100, 38, COLOR_WHITE);
    ssd1306_draw_pixel(100, 37, COLOR_WHITE);
    ssd1306_draw_pixel(100, 36, COLOR_WHITE);
    ssd1306_draw_pixel(100, 35, COLOR_WHITE);
    ssd1306_draw_pixel(100, 34, COLOR_WHITE);
    ssd1306_draw_pixel(100, 33, COLOR_WHITE);
    ssd1306_draw_pixel(100, 32, COLOR_WHITE);
    ssd1306_draw_pixel(100, 31, COLOR_WHITE);
    ssd1306_draw_pixel(100, 30, COLOR_WHITE);
    ssd1306_draw_pixel(100, 29, COLOR_WHITE);
    ssd1306_draw_pixel(99, 28, COLOR_WHITE);
    ssd1306_draw_pixel(99, 27, COLOR_WHITE);
    ssd1306_draw_pixel(99, 26, COLOR_WHITE);
    ssd1306_draw_pixel(99, 25, COLOR_WHITE);
    ssd1306_draw_pixel(98, 25, COLOR_WHITE);
    ssd1306_draw_pixel(98, 24, COLOR_WHITE);
    ssd1306_draw_pixel(98, 23, COLOR_WHITE);
    ssd1306_draw_pixel(97, 22, COLOR_WHITE);
    ssd1306_draw_pixel(97, 21, COLOR_WHITE);
    ssd1306_draw_pixel(96, 21, COLOR_WHITE);
    ssd1306_draw_pixel(96, 20, COLOR_WHITE);
    ssd1306_draw_pixel(96, 19, COLOR_WHITE);
    ssd1306_draw_pixel(95, 19, COLOR_WHITE);
    ssd1306_draw_pixel(95, 18, COLOR_WHITE);
    ssd1306_draw_pixel(95, 17, COLOR_WHITE);
    ssd1306_draw_pixel(94, 17, COLOR_WHITE);
    ssd1306_draw_pixel(94, 16, COLOR_WHITE);
    ssd1306_draw_pixel(94, 15, COLOR_WHITE);
    ssd1306_draw_pixel(93, 15, COLOR_WHITE);
    ssd1306_draw_pixel(93, 14, COLOR_WHITE);
    ssd1306_draw_pixel(92, 14, COLOR_WHITE);
    //id: 7 pixel 10 
    ssd1306_draw_pixel(107, 12, COLOR_WHITE);
    ssd1306_draw_pixel(106, 12, COLOR_WHITE);
    ssd1306_draw_pixel(106, 13, COLOR_WHITE);
    ssd1306_draw_pixel(105, 14, COLOR_WHITE);
    ssd1306_draw_pixel(104, 14, COLOR_WHITE);
    ssd1306_draw_pixel(104, 15, COLOR_WHITE);
    ssd1306_draw_pixel(103, 15, COLOR_WHITE);
    //id: 8 pixel 11 
    ssd1306_draw_pixel(107, 11, COLOR_WHITE);
    ssd1306_draw_pixel(108, 11, COLOR_WHITE);
    ssd1306_draw_pixel(108, 12, COLOR_WHITE);
    ssd1306_draw_pixel(109, 12, COLOR_WHITE);
    ssd1306_draw_pixel(109, 13, COLOR_WHITE);
    ssd1306_draw_pixel(109, 14, COLOR_WHITE);
    ssd1306_draw_pixel(110, 14, COLOR_WHITE);
    ssd1306_draw_pixel(111, 15, COLOR_WHITE);
    ssd1306_draw_pixel(112, 15, COLOR_WHITE);
    ssd1306_draw_pixel(112, 16, COLOR_WHITE);
    //id: 9 pixel 12 
    ssd1306_draw_pixel(120, 11, COLOR_WHITE);
    ssd1306_draw_pixel(120, 12, COLOR_WHITE);
    ssd1306_draw_pixel(119, 12, COLOR_WHITE);
    ssd1306_draw_pixel(119, 13, COLOR_WHITE);
    ssd1306_draw_pixel(118, 13, COLOR_WHITE);
    ssd1306_draw_pixel(118, 14, COLOR_WHITE);
    ssd1306_draw_pixel(118, 15, COLOR_WHITE);
    //id: 10 pixel 13 
    ssd1306_draw_pixel(120, 10, COLOR_WHITE);
    ssd1306_draw_pixel(120, 11, COLOR_WHITE);
    ssd1306_draw_pixel(121, 11, COLOR_WHITE);
    ssd1306_draw_pixel(121, 12, COLOR_WHITE);
    ssd1306_draw_pixel(122, 12, COLOR_WHITE);
    ssd1306_draw_pixel(122, 13, COLOR_WHITE);
    ssd1306_draw_pixel(122, 14, COLOR_WHITE);
    ssd1306_draw_pixel(122, 15, COLOR_WHITE);
    ssd1306_draw_pixel(123, 16, COLOR_WHITE);
    ssd1306_draw_pixel(123, 17, COLOR_WHITE);
    ssd1306_draw_pixel(123, 18, COLOR_WHITE);
    //id: 11 pixel 14 
    ssd1306_draw_pixel(106, 35, COLOR_WHITE);
    ssd1306_draw_pixel(107, 35, COLOR_WHITE);
    ssd1306_draw_pixel(108, 35, COLOR_WHITE);
    ssd1306_draw_pixel(109, 35, COLOR_WHITE);
    ssd1306_draw_pixel(110, 35, COLOR_WHITE);
    ssd1306_draw_pixel(111, 35, COLOR_WHITE);
    ssd1306_draw_pixel(112, 35, COLOR_WHITE);
    ssd1306_draw_pixel(113, 34, COLOR_WHITE);
    ssd1306_draw_pixel(114, 34, COLOR_WHITE);
    ssd1306_draw_pixel(115, 34, COLOR_WHITE);
    ssd1306_draw_pixel(116, 34, COLOR_WHITE);
    ssd1306_draw_pixel(117, 34, COLOR_WHITE);
    ssd1306_draw_pixel(118, 34, COLOR_WHITE);
    ssd1306_draw_pixel(119, 34, COLOR_WHITE);
    ssd1306_draw_pixel(120, 34, COLOR_WHITE);
    ssd1306_display();
}

static void test_ir_interface(void) {
    ir_init();
    ir_receiver_samples_t samples;
    while (1) {
        ir_start_multi_sample();
        ir_read_all_sensors(&samples);
        LOG(LOG_LEVEL_DEBUG, "RIGHT SENSOR VALUE: %d\n", samples.RIGHT_SAMPLE);
        LOG(LOG_LEVEL_DEBUG, "RIGHT DIAG SENSOR VALUE: %d\n", samples.RIGHT_DIAG_SAMPLE);
        LOG(LOG_LEVEL_DEBUG, "LEFT_DIAG SENSOR VALUE: %d\n", samples.LEFT_DIAG_SAMPLE);
        LOG(LOG_LEVEL_DEBUG, "LEFT SENSOR VALUE: %d\n", samples.LEFT_SAMPLE);
        HAL_Delay(500);
    }
}

int main(void) {
    test_setup();
    test_ir_interface();
}