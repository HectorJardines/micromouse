#ifndef _WALL_H
#define _WALL_H

#include "../Inc/drivers/adc.h"
#include "assert_handler.h"

typedef enum {
    WALL_NONE,
    WALL_FRONT,
    WALL_LEFT,
    WALL_RIGHT
} wall_pos_e;

typedef struct {
    uint16_t wall_range;
    wall_pos_e wall_pos;
} wall_info_t;

/**
 * @brief initialize the wall detection API
 * 
 * @return vouid
 */
void wall_init(void);

/**
 * @brief parse IR sensor samples for information about detected wall positions
 */
wall_info_t wall_detect(void);

/**
 * @brief 
 */


#endif