#ifndef _PID_H
#define _PID_H

#include <stdint.h>

#define DIV_4           (2U)
#define DIV_1024        (10U)
#define COUNTS_PER_CM_X1000  (89129U) // 560 counts per rev / 6.283 wheel circumference (scale by 1000)
#define CM_PER_COUNT_X1000   (11U)   // 6.283 wheel circ / 560 counts per rev (scale by 1000)
#define MOTOR_CNT            (2U)

// stop wasting time optimizing early
#define PID_DIST_TO_ENC_COUNT(dist_cm) (((dist_cm) * COUNTS_PER_CM_X1000) / 1000U) // Note: expects dist in CM
#define PID_ENC_COUNT_TO_DIST(count) (((count) * CM_PER_COUNT_X1000) / 1000U) // Note: allowed range of encoder counts is 0-65535

/**
 * @brief PID Initialization function; initializes encoder and pwm modules
 * 
 * @return void
 */
void pid_init(void);

/**
 * @brief Resets the pid angle and dist to a default state
 * 
 * This function resets the angle/goal_distance/correction, error, and previous error terms of 
 * the PID angle/dist structs. 
 * 
 * @return void
 */
void pid_reset(void);

/**
 * @brief Computes the error and correct values and sets necessary PWM
 * 
 * This function reads the left and right encoder counts and computes the 
 * current angle and distance error. Uses these calculations to compute a new 
 * PWM value for left and right motors.
 * 
 * @return void
 */
void pid_update(void);

/**
 * @brief sets the PID dist struct distance goal value
 * 
 * @param distance the distance value to set
 */
void set_pid_goal_dist(int32_t distance);

/**
 * @brief sets the PID angle struct angle goal value
 * 
 * @param angle the angle value to set
 */
void set_pid_goal_angle(int32_t angle);

/**
 * @brief Returns whether dist_goal is within some margin of the actual goal
 * 
 * @return int8_t - boolean value indicating whether goal reached
 */
int8_t pid_done(void);

#endif