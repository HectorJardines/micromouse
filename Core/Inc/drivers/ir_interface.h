#ifndef _IR_INTERFACE_H
#define _IR_INTERFACE_H

#include "io.h"

#define NUM_IR_RECEIVERS (4U)

typedef struct {
    uint16_t LEFT_SAMPLE;
    uint16_t LEFT_DIAG_SAMPLE;
    uint16_t RIGHT_DIAG_SAMPLE;
    uint16_t RIGHT_SAMPLE;
} ir_receiver_samples_t;

typedef enum {
    IR_READ_OK,
    IR_READ_ERROR
} ir_read_status_e;

typedef enum {
    IR_SENSOR_LEFT,
    IR_SENSOR_LEFT_DIAGONAL,
    IR_SENSOR_RIGHT_DIAGONAL,
    IR_SENSOR_RIGHT
} ir_sensor_pos_e;

// typedef uint16_t ir_rcv_vals[NUM_IR_RECEIVERS];

/**
 * @brief Initialize IR interface
 */
void ir_init(void);

/**
 * @brief Enables the IR emitter for the specified sensor position
 * 
 * @param pos Position of IR emitter to be enabled
 * @return void
 */
void ir_start_single_sample(ir_sensor_pos_e pos);

/**
 * @brief Enables all IR emitters
 * 
 * Will likely be used as the primary method for enabling IR emitters.
 * Deciding on whether to toggle emitters on and off as needed, or having the constantly
 * enbaled for quicker sampling and updated values... Could use the DMA interrupts to set a flag
 * that signals that fresh values are loaded, this would allow us to know how long to wait 
 * before reading samples after starting a sample.
 * 
 * @return void
 */
void ir_start_multi_sample(void);

/**
 * @brief Read a single sample of the adc channel corresponding to the ir_sensor_pos_e
 * 
 * @param sensor uint16_t pointer to which the sample value will be stored
 * @param pos sensor position enum 
 */
ir_read_status_e ir_read_single_sensor(ir_receiver_samples_t *sensors, ir_sensor_pos_e pos);

/**
 * @brief Read all ADC channels samples of the IR sensors
 * 
 * @param sensors array of uint16_t values that hold channel samples
 * @note Though unlikely, there is a chance of emitter collision, e.g. left diagonal emitter
 * signal may be picked up on left receiver or vice versa.
 */
ir_read_status_e ir_read_all_sensors(ir_receiver_samples_t *receiver_values);

#endif