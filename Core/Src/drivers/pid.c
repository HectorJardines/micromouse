#include "../inc/drivers/pid.h"
#include "../inc/drivers/tb6612fng.h"
#include "../inc/drivers/encoder.h"

// 240 encoder counts per wheel rotation (360 deg motor rotation / 30 deg per encoder count => 12 counts per motor rotation * 20:1 gear ratio => 240 counts per wheel rotation)

struct pid_cntl_dist {
    int32_t err;
    int32_t prev_err;
    int32_t kP;                 /* proportional constant (tune this) */
    int32_t kD;                 /* derivative constant (tune this) */
    int16_t goal_dist;
    int16_t goal_correction;
};

struct pid_cntl_angle {
    int32_t kPw;
    int32_t kDw;
    int16_t err;
    int16_t prev_err;
    int16_t goal_angle;
    int16_t angle_correction;
};

static struct pid_cntl_dist pid_dist = {.err = 0, .prev_err = 0, .kD = 0, .kP = 0, .goal_dist = 0};
static struct pid_cntl_angle pid_angle = {.err = 0, .prev_err = 0, .kPw = 0, .kDw = 0, .goal_angle = 0};

void reset_pid(void)
{

}

void update_pid(void)
{
    int16_t encoder_cnt_left = encoder_read_left_count();
    int16_t encoder_cnt_right = encoder_read_right_count();

    pid_dist.goal_correction = (pid_dist.kP * pid_dist.err) + (pid_dist.kD * ((pid_dist.err - pid_dist.prev_err)));
    pid_angle.angle_correction = (pid_angle.kPw * pid_angle.err) + (pid_angle.kDw * (pid_dist.err - pid_dist.prev_err));
    
    // error is considered as the goal - average of the two encoder counts
    pid_dist.prev_err = pid_dist.err;
    // pid_dist.err is at most 32767 (2 * MAX_INT16_T / 2) and at least -32767
    pid_dist.err = pid_dist.goal_dist - ((encoder_cnt_left + encoder_cnt_right) / 2);
    pid_angle.prev_err = pid_angle.err;

    pid_angle.err = pid_angle.goal_angle - (encoder_cnt_left - encoder_cnt_right);

    int16_t right_motor_pwm = pid_dist.goal_correction - pid_angle.angle_correction;
    int16_t left_motor_pwm = pid_dist.goal_correction + pid_angle.angle_correction;
    
    // set left and right motor speeds and directions
    tb6612fng_dir_e right_motor_dir = right_motor_pwm < 0 ? dir_reverse : right_motor_pwm > 0 ? dir_forward : dir_stop;
    tb6612fng_set_dir(tb6612fng_motor_right, right_motor_dir);
    tb6612fng_set_speed(tb6612fng_motor_right, right_motor_pwm);

    tb6612fng_dir_e left_motor_dir = left_motor_pwm < 0 ? dir_reverse : left_motor_pwm > 0 ? dir_forward : dir_stop;
    tb6612fng_set_dir(tb6612fng_motor_left, left_motor_dir);
    tb6612fng_set_speed(tb6612fng_motor_left, left_motor_pwm);
}

void set_pid_goal_dist(int16_t distance)
{
    // calculated as the sum of left and right encoder counts
    // compare against the goal distance and update error accordingly
    pid_dist.goal_dist = distance;
}

void set_pid_goal_angle(int16_t angle)
{
    // calculated as the difference between left and right encoder counts
    // compare the value against the "goal" angle and update the error accordingly
    pid_angle.goal_angle = angle;
}

int8_t pid_done(void)
{
    return -1;
}
