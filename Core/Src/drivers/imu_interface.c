#include "imu_interface.h"
#include "spi.h"
#include "assert_handler.h"
#include "stm32f1xx_hal.h"
#include "log.h"

#define NUM_BIAS_SAMPLES (1024U)
#define DIV_1024 (10U)

static struct bmi160_dev imu_cnf;
struct bmi160_dev actual_cnf = {.read = spi_read_data, .write = spi_write_data, .delay_ms = HAL_Delay, .intf = BMI160_SPI_INTF, .id = BMI160_CHIP_ID};
static struct imu_gyro_z_data gyro_z = {.bias = 0, .gyro_z_calibrated = 0};

/****************************
 *  STATIC DECLARATIONS
 ****************************/
static void imu_configure(void);
static int8_t imu_compare_cnf(struct bmi160_dev *expected, struct bmi160_dev *actual);
static int8_t imu_compute_gyro_z_bias(void);
static int8_t imu_restore_sensor_cfg(void);

/*************************
 *  IMU API DEFINITIONS
 *************************/
static bool initialzed = false;
void imu_init(void) {
    ASSERT(!initialzed, ASSERT_DRIVER_LEVEL);
    spi_init();
    imu_configure();
    int8_t rslt = imu_compare_cnf(&imu_cnf, &actual_cnf);
    if (rslt)
        LOG("INCORRECT CONFIGURATION...\n");
    imu_compute_gyro_z_bias();
    initialzed = true;
}

int8_t imu_self_test_gyro(void) {
    int8_t res = bmi160_perform_self_test(BMI160_GYRO_ONLY, &imu_cnf);
    // self test API resets the sensor; reconfigure
    res = imu_restore_sensor_cfg();
    if (res == BMI160_W_GYRO_SELF_TEST_FAIL) {
        LOG("GYRO SELF TEST ROUTINE FAILED...\n");
    }
    return res;
}

int8_t imu_self_test_accel(void) {
    int8_t res = bmi160_perform_self_test(BMI160_ACCEL_ONLY, &imu_cnf);
    res = imu_restore_sensor_cfg();
    if (res == BMI160_W_ACCEl_SELF_TEST_FAIL)
        LOG("ACCELEROMETER SELF TEST FAILED...\n");
    return res;
}

int8_t imu_get_gyro_z(float *angle_z, float sampling_delta) {
    struct bmi160_sensor_data gyro;
    int8_t res = bmi160_get_sensor_data(BMI160_GYRO_ONLY, NULL, &gyro, &imu_cnf);
    *angle_z += (gyro.z - gyro_z.bias);
    return res;
}

/***************************
 *  STATIC DEFINITIONS
 ***************************/

/**
 * @brief compute the bias term of z axis gyro samples over 1024 samples
 */
static int8_t imu_compute_gyro_z_bias(void) {
    int8_t res = 0;
    int32_t sum_samples = 0;
    struct bmi160_sensor_data gyro;
    for (int i = 0; i < NUM_BIAS_SAMPLES; i++) {
        res = bmi160_get_sensor_data(BMI160_GYRO_ONLY, NULL, &gyro, &imu_cnf);
        sum_samples += gyro.z;
    }
    gyro_z.bias = sum_samples >> DIV_1024;
    return res;
}

/**
 * @brief compare the cofigured sensor settings vs the expected sensor settings
 */
static int8_t imu_compare_cnf(struct bmi160_dev *expected, struct bmi160_dev *actual) {
    
    int8_t rslt = true;
    rslt = bmi160_get_sens_conf(actual);
    if (rslt)
        return rslt;
    // Compare accel
    rslt = expected->accel_cfg.bw == actual->accel_cfg.bw && expected->accel_cfg.odr == actual->accel_cfg.odr && expected->accel_cfg.range == actual->accel_cfg.range;
    LOG("ACTUAL ACCEL BW, ODR, RANGE: %d, %d, %d | EXPECTED ACCEL BW, ODR, RANGE: %d, %d, %d\n", actual->accel_cfg.bw, actual->accel_cfg.odr, actual->accel_cfg.range, expected->accel_cfg.bw, expected->accel_cfg.odr, expected->accel_cfg.range);
    if (!rslt)
        return -1;
    // compare gyro
    rslt = expected->gyro_cfg.bw == actual->gyro_cfg.bw && expected->gyro_cfg.odr == actual->gyro_cfg.odr && expected->gyro_cfg.range == actual->gyro_cfg.range;
    LOG("ACTUAL gyro BW, ODR, RANGE: %d, %d, %d | EXPECTED gyro BW, ODR, RANGE: %d, %d, %d\n", actual->gyro_cfg.bw, actual->gyro_cfg.odr, actual->gyro_cfg.range, expected->gyro_cfg.bw, expected->gyro_cfg.odr, expected->gyro_cfg.range);
    if (!rslt)
        return -1;

    return 0;
}

/**
 * @brief configure bmi160 registers to support interrupt driven access
 */
static void imu_configure(void) {
    // 1. Set interface (SPI)
    imu_cnf.intf = BMI160_SPI_INTF;
    imu_cnf.any_sig_sel = BMI160_BOTH_ANY_SIG_MOTION_DISABLED;

    // 2. set spi read/write/delay functions
    imu_cnf.read = spi_read_data;
    imu_cnf.write = spi_write_data;
    imu_cnf.delay_ms = HAL_Delay;

    // 3. init device and retrieve chip id
    int8_t res = bmi160_init(&imu_cnf);
    ASSERT(!res, ASSERT_DRIVER_LEVEL);

    // 4. Set interrupts on data ready
    // set output enable; set output mode = push pull; set output type active high; set edge ctrl = edge trigger; clear input enable; int latching only matters in low power
    struct bmi160_int_pin_settg int_pin_cnf = {.output_en = ENABLE, .output_mode = 0, .output_type = 1, .edge_ctrl = 1, .input_en = 0};
    struct bmi160_int_settg int_settg = {.int_type = BMI160_ACC_GYRO_DATA_RDY_INT, .int_channel = BMI160_INT_CHANNEL_1, .int_pin_settg = int_pin_cnf, .fifo_full_int_en = DISABLE, .fifo_wtm_int_en = DISABLE};
    res = bmi160_set_int_config(&int_settg, &imu_cnf);
    ASSERT(!res, ASSERT_DRIVER_LEVEL);

    // 5. Configure accel
    struct bmi160_cfg accel_cnf = {.power = BMI160_ACCEL_NORMAL_MODE, .bw = BMI160_ACCEL_BW_NORMAL_AVG4, .odr = BMI160_ACCEL_ODR_800HZ, .range = BMI160_ACCEL_RANGE_8G};
    imu_cnf.accel_cfg = accel_cnf;

    // 6. Configure gyro
    struct bmi160_cfg gyro_cnf = {.power = BMI160_GYRO_NORMAL_MODE, .bw = BMI160_GYRO_BW_NORMAL_MODE, .odr = BMI160_GYRO_ODR_800HZ, .range = BMI160_GYRO_RANGE_2000_DPS};
    imu_cnf.gyro_cfg = gyro_cnf;
    res = bmi160_set_sens_conf(&imu_cnf);
    ASSERT(!res, ASSERT_DRIVER_LEVEL);
}

/**
 * @brief Restores accel/gyro sensors after self-test
 */
static int8_t imu_restore_sensor_cfg(void) {
    int8_t res = 0;
    // 5. Configure accel
    struct bmi160_cfg accel_cnf = {.power = BMI160_ACCEL_NORMAL_MODE, .bw = BMI160_ACCEL_BW_NORMAL_AVG4, .odr = BMI160_ACCEL_ODR_800HZ, .range = BMI160_ACCEL_RANGE_8G};
    imu_cnf.accel_cfg = accel_cnf;

    // 6. Configure gyro
    struct bmi160_cfg gyro_cnf = {.power = BMI160_GYRO_NORMAL_MODE, .bw = BMI160_GYRO_BW_NORMAL_MODE, .odr = BMI160_GYRO_ODR_800HZ, .range = BMI160_GYRO_RANGE_2000_DPS};
    imu_cnf.gyro_cfg = gyro_cnf;
    res = bmi160_set_sens_conf(&imu_cnf);
    ASSERT(!res, ASSERT_DRIVER_LEVEL);

    res = imu_compare_cnf(&imu_cnf, &actual_cnf);
    if (res)
        LOG("INCORRECT CONFIGURATION AFTER SELF TEST\n");

    return res;
}

