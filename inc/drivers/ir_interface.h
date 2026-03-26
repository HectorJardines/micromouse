#ifndef _IR_INTERFACE_H
#define _IR_INTERFACE_H

#include "io.h"

typedef enum {
    IR_READ_OK,
    IR_READ_ERROR
} ir_read_status_e;

void ir_init(void);
ir_read_status_e ir_read_single_sensor(uint8_t *sensor);
ir_read_status_e ir_read_multiple_sensors(uint8_t *sensors);

#endif