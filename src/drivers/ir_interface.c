#include "ir_interface.h"

void ir_init(void);
ir_read_status_e ir_read_single_sensor(uint8_t *sensor);
ir_read_status_e ir_read_multiple_sensors(uint8_t *sensors);