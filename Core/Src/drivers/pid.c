#include "../inc/drivers/pid.h"
#include "../inc/drivers/tb6612fng.h"
#include "../inc/drivers/encoder.h"
#include "../Inc/common/log.h"
#include "../Inc/common/assert_handler.h"
#include "../Inc/drivers/imu_interface.h"

#define GOAL_MARGIN_BEHIND  (500U)    // this number should encode mouse roughly 5cm behind center of goal
#define GOAL_MARGIN_AHEAD   (-500U)   // this number should encode mouse roughly 5cm ahead of center of goal

#define DERIV_CONST_DIST    (5U)
#define PROP_CONST_DIST     (5U)
#define DERIV_CONST_ANG     (5U)
#define PROP_CONST_ANG      (10U)
#define MAX_TIM_CNT_VAL     (65535U)

// 7 Pulses Per Revolution * 4 signals per revolutions (2 on enc A and 2 on enc B) = 28 counters per revolution
// with 20:1 gear ratio we have 28 counts (per rev of input shaft) * 20 (gear ratio) = 560 counts per rev of output shaft
/**
 * Using the specs from the motor, you would need to find the encoder counts per revolution (of the output shaft). 
 * Then, you know that corresponds to 360 degrees of wheel rotation, which means the distance travelled is the circumference 
 * of the wheel (2 * pi * r_wheel). To figure out how many encoder ticks correspond to the distance you wanna go, 
 * just multiply the distance by the counts / distance you calculated above. Hope that helps!
 */

struct pid_cntl_dist {
    int32_t err;
    int32_t prev_err;
    int32_t kP;                 /* proportional constant (tune this) */
    int32_t kD;                 /* derivative constant (tune this) */
    int32_t goal_dist;
    int32_t goal_correction;
    // TODO: CONSIDER SEPARATING THESE VALUES OUT INTO A DIFF STRUCT FOR MOTORS
    int32_t total_ticks;
    int32_t prev_tick_sample_l;
    int32_t prev_tick_sample_r;
    int16_t duty_cycle_l;
    int16_t duty_cycle_r;
    tb6612fng_dir_e motor_dir_left;
    tb6612fng_dir_e motor_dir_right;
};

struct pid_cntl_angle {
    int32_t kPw;
    int32_t kDw;
    int32_t err;
    int32_t prev_err;
    int32_t goal_angle;
    int32_t angle_correction;
    int32_t total_ticks;
    int32_t prev_tick_sample;
    
    int32_t total_difference;
};

// we can have .kD/P be a number between 0-100, do some kind of scaling and division to avoid using floating point numbers
static struct pid_cntl_dist pid_dist = {.err = 0, .prev_err = 0, .kD = DERIV_CONST_DIST, .kP = PROP_CONST_DIST, .goal_dist = 0, .goal_correction = 0, .total_ticks = 0, .prev_tick_sample_l = 0, .prev_tick_sample_r = 0};
static struct pid_cntl_angle pid_angle = {.err = 0, .prev_err = 0, .kPw = DERIV_CONST_ANG, .kDw = PROP_CONST_ANG, .goal_angle = 0, .angle_correction = 0, .total_ticks = 0, .prev_tick_sample = 0};

static bool initialized = false;
void pid_init(void) {
    ASSERT(!initialized, ASSERT_DRIVER_LEVEL);
    encoder_init();
    imu_init();
    pwm_init();
    initialized = true;
}

void pid_reset(void)
{
    pid_dist.goal_dist = 0;
    pid_angle.goal_angle = 0;

    pid_dist.err = 0;
    pid_angle.err = 0;

    pid_dist.prev_err = 0;
    pid_angle.prev_err = 0;

    pid_dist.goal_correction = 0;
    pid_angle.angle_correction = 0;

    pid_dist.total_ticks = 0;
    pid_dist.duty_cycle_l = 0;
    pid_dist.duty_cycle_r = 0;
    pid_dist.prev_tick_sample_l = 0;
    pid_dist.prev_tick_sample_r = 0;
    pid_dist.motor_dir_left = dir_stop;
    pid_dist.motor_dir_right = dir_stop;
}

/**
 * @brief Scale the computed duty cycle value to fit in TIM2->ARR
 */
static int16_t pid_clamp_duty_cycle(int32_t pid_duty_val) {
    // max duty cycle of 3199
    int32_t scaled_pwm = pid_duty_val;
    scaled_pwm = scaled_pwm >> DIV_4;
    return scaled_pwm;
}

/**
 * @brief Checks current and previous encoder tick samples and evaluates under/overflow and updates global tick counter
 * 
 * @param curr_enc_tick_sample Encoder count value read from encoder
 * @param encoder Indicates the encoder (left or right) to which the sample corresponds
 */
static void pid_update_global_tick_cnt(uint32_t curr_enc_tick_sample, encoder_e encoder) {
    tb6612fng_dir_e motor_dir = encoder_left ? pid_dist.motor_dir_left : pid_dist.motor_dir_right;
    int32_t prev_enc_tick_sample = encoder == encoder_left ? pid_dist.prev_tick_sample_l : pid_dist.prev_tick_sample_r;
    int32_t count = 0;
    // overflow occurs when our motor direction is forward and prev_sample > current_sample
    if ((motor_dir == dir_forward) && (prev_enc_tick_sample > curr_enc_tick_sample)) {
        count = ((curr_enc_tick_sample - prev_enc_tick_sample) % MAX_TIM_CNT_VAL);
        pid_dist.total_ticks += count;
        if (encoder == encoder_left)
            pid_angle.total_difference += count;
        else
            pid_angle.total_difference -= count;
    }
    // underflow occurs when motor direction is reverse and prev_sample < current_sample
    else if ((motor_dir == dir_reverse) && (prev_enc_tick_sample < curr_enc_tick_sample)) {
        count = ((prev_enc_tick_sample - curr_enc_tick_sample) % MAX_TIM_CNT_VAL);
        pid_dist.total_ticks -= count;
        if (encoder == encoder_left)
            pid_angle.total_difference -= count;
        else
            pid_angle.total_difference += count;
    }
    else if (motor_dir == dir_forward) {
        count = (curr_enc_tick_sample - prev_enc_tick_sample);
        pid_dist.total_ticks += count;
        if (encoder == encoder_left)
            pid_angle.total_difference += count;
        else
            pid_angle.total_difference -= count;
    }
    else if (motor_dir == dir_reverse) {
        count = (prev_enc_tick_sample - curr_enc_tick_sample);
        pid_dist.total_ticks -= count;
        if (encoder == encoder_left)
            pid_angle.total_difference += count;
        else
            pid_angle.total_difference -= count;
    }
}

/**
 * @brief Sets the updated motor directions and speeds based on pid update function
 */
static void pid_update_motor_ctl(void) {
    // set motor directions
    tb6612fng_set_dir(tb6612fng_motor_right, pid_dist.motor_dir_right);
    tb6612fng_set_dir(tb6612fng_motor_left, pid_dist.motor_dir_left);

    // set motor speeds
    tb6612fng_set_speed(tb6612fng_motor_right, pid_dist.duty_cycle_r);
    tb6612fng_set_speed(tb6612fng_motor_left, pid_dist.duty_cycle_l);
    LOG("RIGHT MOTOR VALUES - PWM: %d, DIR: %d", pid_dist.duty_cycle_r, pid_dist.motor_dir_right);
    LOG("LEFT MOTOR VALUES - PWM: %d, DIR: %d", pid_dist.duty_cycle_l, pid_dist.motor_dir_left);
}

void pid_update(void)
{
    // change this to be interrupt driven
    uint32_t encoder_cnt_left = encoder_read_left_count();
    pid_update_global_tick_cnt(encoder_cnt_left, encoder_left);
    uint32_t encoder_cnt_right = encoder_read_right_count();
    pid_update_global_tick_cnt(encoder_cnt_right, encoder_right);
    float gyro_z = 0;
    // imu_get_gyro_z(&gyro_z, 1);

    // error is considered as the goal - average of the two encoder counts
    pid_dist.prev_err = pid_dist.err;
    // pid_dist.err is at most 65535 (2 * UINT16_MAX / 2) and at least 0
    pid_dist.err = pid_dist.goal_dist - (pid_dist.total_ticks >> 2);

    pid_angle.prev_err = pid_angle.err;
    // again at most we will have 65535 (when goal_angle = enc_left = 0 and enc_right = 65535)
    pid_angle.err = pid_angle.goal_angle - (pid_angle.total_difference);

    // at most will be 13106, when pid_dist.err = 65535 and pid_dist.prev_err = 0
    pid_dist.goal_correction = ((pid_dist.kP * pid_dist.err) / 100U) + ((pid_dist.kD * ((pid_dist.err - pid_dist.prev_err))) / 100U);
    pid_angle.angle_correction = ((pid_angle.kPw * pid_angle.err) / 100U) + ((pid_angle.kDw * (gyro_z)) / 100U);

    pid_dist.duty_cycle_r = pid_clamp_duty_cycle(pid_dist.goal_correction - pid_angle.angle_correction);
    pid_dist.duty_cycle_l = pid_clamp_duty_cycle(pid_dist.goal_correction + pid_angle.angle_correction);

    // set left and right motor speeds and directions
    pid_dist.motor_dir_right = pid_dist.duty_cycle_r < 0 ? dir_reverse : pid_dist.duty_cycle_r > 0 ? dir_forward : dir_stop;
    pid_dist.motor_dir_left = pid_dist.duty_cycle_l < 0 ? dir_reverse : pid_dist.duty_cycle_l > 0 ? dir_forward : dir_stop;

    pid_update_motor_ctl();
    LOG("PID DIST: ERR = %d, TOTAL_CNTS = %d\n", pid_dist.err, pid_dist.total_ticks);
    LOG("PID ANGLE: ERR = %d, TOTAL DIFF CNTS = %d\n", pid_angle.err, pid_angle.total_difference);
}

void set_pid_goal_dist(int32_t distance)
{
    // calculated as the sum of left and right encoder counts
    // compare against the goal distance and update error accordingly
    pid_dist.goal_dist = distance;
}

void set_pid_goal_angle(int32_t angle)
{
    // calculated as the difference between left and right encoder counts
    // compare the value against the "goal" angle and update the error accordingly
    pid_angle.goal_angle = angle;
}

int8_t pid_done(void)
{
    // return pid_dist.err <= GOAL_MARGIN_BEHIND && pid_dist.err >= GOAL_MARGIN_AHEAD;
    int8_t pid_status = (pid_dist.total_ticks >> 2) <= (pid_dist.goal_dist + GOAL_MARGIN_BEHIND) && (pid_dist.total_ticks >> 2) >= (pid_dist.goal_dist + GOAL_MARGIN_AHEAD);
    if (pid_status) {
        pid_reset();
        pid_update_motor_ctl();
    }
    return pid_status;
}
