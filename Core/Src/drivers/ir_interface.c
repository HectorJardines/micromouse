#include "ir_interface.h"
#include "assert_handler.h"
#include "../Inc/drivers/adc.h"

static bool initialized = false;
void ir_init(void) {
    ASSERT(!initialized, ASSERT_DRIVER_LEVEL);
    adc_init();
    initialized = true;
}

void ir_start_single_sample(ir_sensor_pos_e pos) {

}

void ir_start_multi_sample(void) {
    
}

ir_read_status_e ir_read_single_sensor(ir_receiver_samples_t *sensors, ir_sensor_pos_e pos) {
    if (pos < IR_SENSOR_LEFT || pos > IR_SENSOR_RIGHT)
        ASSERT(0, ASSERT_DRIVER_LEVEL);

    adc_channel_values_t ir_rcv_samples;
    adc_sample_channels(&ir_rcv_samples);
    // each position enum directly corresponds to the index of the IR sensor
    switch (pos) {
        case IR_SENSOR_LEFT:
            sensors->LEFT_SAMPLE = ir_rcv_samples[pos];
            break;
        case IR_SENSOR_LEFT_DIAGONAL:
            sensors->LEFT_DIAG_SAMPLE = ir_rcv_samples[pos];
            break;
        case IR_SENSOR_RIGHT_DIAGONAL:
            sensors->RIGHT_DIAG_SAMPLE = ir_rcv_samples[pos];
            break;
        case IR_SENSOR_RIGHT:
            sensors->RIGHT_SAMPLE = ir_rcv_samples[pos];
            break;
    }
}

ir_read_status_e ir_read_all_sensors(ir_receiver_samples_t *receiver_values) {
    adc_channel_values_t ir_rcv_vals;
    adc_sample_channels(ir_rcv_vals);
    receiver_values->LEFT_SAMPLE = ir_rcv_vals[IR_SENSOR_LEFT];
    receiver_values->LEFT_DIAG_SAMPLE = ir_rcv_vals[IR_SENSOR_LEFT_DIAGONAL];
    receiver_values->RIGHT_DIAG_SAMPLE = ir_rcv_vals[IR_SENSOR_RIGHT_DIAGONAL];
    receiver_values->RIGHT_SAMPLE = ir_rcv_vals[IR_SENSOR_RIGHT];
}

