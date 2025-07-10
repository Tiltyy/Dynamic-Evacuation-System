#ifndef SGP30_DRIVER_H
#define SGP30_DRIVER_H

#include <stdint.h>

// SGP30 I2C device address
// SGP30 I2C设备地址
#define SGP30_I2C_ADDRESS 0x58

// SGP30 commands
// SGP30命令
#define SGP30_INIT_AIR_QUALITY 0x2003
#define SGP30_MEASURE_AIR_QUALITY 0x2008
#define SGP30_GET_BASELINE 0x2015
#define SGP30_SET_BASELINE 0x201E
#define SGP30_MEASURE_TEST 0x2032
#define SGP30_GET_FEATURE_SET_VERSION 0x202F
#define SGP30_MEASURE_RAW_SIGNALS 0x2050
#define SGP30_SET_HUMIDITY 0x2061

// Function prototypes
// 函数原型
/**
 * @brief Initializes the SGP30 sensor.
 * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-0").
 * @return File descriptor if successful, -1 otherwise.
 */
int sgp30_init(const char *i2c_dev_path);

/**
 * @brief Reads TVOC and eCO2 values from the SGP30 sensor.
 * @param fd File descriptor of the I2C device.
 * @param tvoc Pointer to store the TVOC value.
 * @param eco2 Pointer to store the eCO2 value.
 * @return 0 if successful, -1 otherwise.
 */
int sgp30_read_air_quality(int fd, uint16_t *tvoc, uint16_t *eco2);

/**
 * @brief Closes the SGP30 I2C device.
 * @param fd File descriptor of the I2C device.
 */
void sgp30_close(int fd);

#endif // SGP30_DRIVER_H


