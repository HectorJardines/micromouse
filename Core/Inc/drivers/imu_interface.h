#ifndef _BMI_160_INT_H
#define _BMI_160_INT_H

#include "bmi160.h"

struct imu_gyro_z_data {
    int16_t bias;
    float gyro_z_calibrated;
};

/**
 * @brief Initialize the BMI160 IMU using the bmi160 API
 */
void imu_init(void);

/**
 * @brief 
 */
int8_t imu_self_test_gyro(void);

/**
 * @brief
 */
int8_t imu_self_test_accel(void);

/**
 * @brief
 */
int8_t imu_read_sensor_data(struct bmi160_sensor_data *accel, struct bmi160_sensor_data *gyro);

/**
 * @brief
 */
int8_t imu_get_accel_pitch_roll(float *pitch, float *roll);

/**
 * @brief
 */
int8_t imu_get_gyro_z(float *angle_z, float sampling_delta);

#endif