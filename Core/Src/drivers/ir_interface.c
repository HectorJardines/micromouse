#include "ir_interface.h"
#include "assert_handler.h"
#include "../Inc/drivers/adc.h"
#include "../Inc/common/defines.h"

struct ir_emitters_t {
    uint8_t left_en;
    uint8_t left_diag_en;
    uint8_t right_diag_en;
    uint8_t right_en;
};
static struct ir_emitters_t emitters = {.left_en = DISABLE, .left_diag_en = DISABLE, .right_diag_en = DISABLE, .right_en = DISABLE};

static bool initialized = false;
void ir_init(void) {
    ASSERT(!initialized, ASSERT_DRIVER_LEVEL);
    adc_init();
    initialized = true;
}

void ir_start_single_sample(ir_sensor_pos_e pos) {
    if (pos < IR_SENSOR_LEFT || pos > IR_SENSOR_RIGHT)
        return;

    switch (pos) {
        case IR_SENSOR_LEFT:
            io_set_out(IO_IR_EMIT_4, HIGH);
            emitters.left_en = ENABLE;
            HAL_Delay(20); // delay to ensure emitter has been on long enough for adc to sample
            break;
        case IR_SENSOR_LEFT_DIAGONAL:
            io_set_out(IO_IR_EMIT_3, HIGH);
            emitters.left_diag_en = ENABLE;
            HAL_Delay(20);
            break;
        case IR_SENSOR_RIGHT_DIAGONAL:
            io_set_out(IO_IR_EMIT_2, HIGH);
            emitters.right_diag_en = ENABLE;
            HAL_Delay(20);
            break;
        case IR_SENSOR_RIGHT:
            io_set_out(IO_IR_EMIT_1, HIGH);
            emitters.right_en = ENABLE;
            HAL_Delay(20);
            break;
    }
}

void ir_start_multi_sample(void) {
    io_set_out(IO_IR_EMIT_4, HIGH);
    emitters.left_en = ENABLE;
    io_set_out(IO_IR_EMIT_3, HIGH);
    emitters.left_diag_en = ENABLE;
    io_set_out(IO_IR_EMIT_2, HIGH);
    emitters.right_diag_en = ENABLE;
    io_set_out(IO_IR_EMIT_1, HIGH);
    emitters.right_en = ENABLE;
    HAL_Delay(20);
}

ir_read_status_e ir_read_single_sensor(ir_receiver_samples_t *sensors, ir_sensor_pos_e pos) {
    if (pos < IR_SENSOR_LEFT || pos > IR_SENSOR_RIGHT)
        ASSERT(0, ASSERT_DRIVER_LEVEL);

    adc_channel_values_t ir_rcv_samples = {0, 0, 0, 0};
    adc_sample_channels(ir_rcv_samples);
    // each position enum directly corresponds to the index of the IR sensor
    switch (pos) {
        case IR_SENSOR_LEFT:
            sensors->LEFT_SAMPLE = ir_rcv_samples[pos];
            if (emitters.left_en) {
                io_set_out(IO_IR_EMIT_4, LOW);
                emitters.left_en = DISABLE;
            }
            break;
        case IR_SENSOR_LEFT_DIAGONAL:
            sensors->LEFT_DIAG_SAMPLE = ir_rcv_samples[pos];
            if (emitters.left_diag_en) {
                io_set_out(IO_IR_EMIT_3, LOW);
                emitters.left_diag_en = DISABLE;
            }
            break;
        case IR_SENSOR_RIGHT_DIAGONAL:
            sensors->RIGHT_DIAG_SAMPLE = ir_rcv_samples[pos];
            if (emitters.right_diag_en) {
                io_set_out(IO_IR_EMIT_2, LOW);
                emitters.right_diag_en = DISABLE;
            }
            break;
        case IR_SENSOR_RIGHT:
            sensors->RIGHT_SAMPLE = ir_rcv_samples[pos];
            if (emitters.right_en) {
                io_set_out(IO_IR_EMIT_1, LOW);
                emitters.right_en = DISABLE;
            }
            break;
    }
    return IR_READ_OK;
}


ir_read_status_e ir_read_all_sensors(ir_receiver_samples_t *receiver_values) {
    adc_channel_values_t ir_rcv_vals;
    adc_sample_channels(ir_rcv_vals);
    receiver_values->LEFT_SAMPLE = ir_rcv_vals[IR_SENSOR_LEFT];
    receiver_values->LEFT_DIAG_SAMPLE = ir_rcv_vals[IR_SENSOR_LEFT_DIAGONAL];
    receiver_values->RIGHT_DIAG_SAMPLE = ir_rcv_vals[IR_SENSOR_RIGHT_DIAGONAL];
    receiver_values->RIGHT_SAMPLE = ir_rcv_vals[IR_SENSOR_RIGHT];

    io_set_out(IO_IR_EMIT_1, LOW);
    emitters.left_en = DISABLE;
    io_set_out(IO_IR_EMIT_2, LOW);
    emitters.left_diag_en = DISABLE;
    io_set_out(IO_IR_EMIT_3, LOW);
    emitters.right_diag_en = DISABLE;
    io_set_out(IO_IR_EMIT_4, LOW);
    emitters.right_en = DISABLE;
    HAL_Delay(20);

    return IR_READ_OK;
}

