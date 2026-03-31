#ifndef _PID_H
#define _PID_H

#include <stdint.h>

void reset_pid(void);
void update_pid(void);
void set_pid_goal_dist(int16_t distance);
void set_pid_goal_angle(int16_t angle);
int8_t pid_done(void);

#endif