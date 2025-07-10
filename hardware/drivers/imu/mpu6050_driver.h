#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <stdint.h>

// MPU6050 I2C address
// MPU6050 I2C地址
#define MPU6050_ADDRESS_AD0_LOW  0x68 // AD0 pin low
#define MPU6050_ADDRESS_AD0_HIGH 0x69 // AD0 pin high

#define MPU6050_ADDRESS MPU6050_ADDRESS_AD0_LOW // Default address for main.c

// MPU6050 Registers
// MPU6050寄存器
#define MPU6050_REG_SMPLRT_DIV    0x19
#define MPU6050_REG_CONFIG        0x1A
#define MPU6050_REG_GYRO_CONFIG   0x1B
#define MPU6050_REG_ACCEL_CONFIG  0x1C
#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_ACCEL_XOUT_L  0x3C
#define MPU6050_REG_ACCEL_YOUT_H  0x3D
#define MPU6050_REG_ACCEL_YOUT_L  0x3E
#define MPU6050_REG_ACCEL_ZOUT_H  0x3F
#define MPU6050_REG_ACCEL_ZOUT_L  0x40
#define MPU6050_REG_TEMP_OUT_H    0x41
#define MPU6050_REG_TEMP_OUT_L    0x42
#define MPU6050_REG_GYRO_XOUT_H   0x43
#define MPU6050_REG_GYRO_XOUT_L   0x44
#define MPU6050_REG_GYRO_YOUT_H   0x45
#define MPU6050_REG_GYRO_YOUT_L   0x46
#define MPU6050_REG_GYRO_ZOUT_H   0x47
#define MPU6050_REG_GYRO_ZOUT_L   0x48
#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_WHO_AM_I      0x75

// MPU6050 Full Scale Ranges
// MPU6050满量程范围
#define MPU6050_ACCEL_FS_2G   0x00
#define MPU6050_ACCEL_FS_4G   0x08
#define MPU6050_ACCEL_FS_8G   0x10
#define MPU6050_ACCEL_FS_16G  0x18

#define MPU6050_GYRO_FS_250   0x00
#define MPU6050_GYRO_FS_500   0x08
#define MPU6050_GYRO_FS_1000  0x10
#define MPU6050_GYRO_FS_2000  0x18

// Structure to hold MPU6050 data
// 用于存储MPU6050数据的结构体
typedef struct {
    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x, gyro_y, gyro_z;
    int16_t temperature;
} MPU6050_Data_t;

// Function prototypes
// 函数原型
/**
 * @brief Initializes the MPU6050 sensor.
 * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-0").
 * @param address I2C address of the MPU6050 (MPU6050_ADDRESS_AD0_LOW or MPU6050_ADDRESS_AD0_HIGH).
 * @return File descriptor if successful, -1 otherwise.
 */
int mpu6050_init(const char *i2c_dev_path, uint8_t address);

/**
 * @brief Reads all sensor data (accelerometer, gyroscope, temperature) from MPU6050.
 * @param fd File descriptor of the I2C device.
 * @param data Pointer to MPU6050_Data_t structure to store the read data.
 * @return 0 if successful, -1 otherwise.
 */
int mpu6050_read_data(int fd, MPU6050_Data_t *data);

/**
 * @brief Closes the MPU6050 I2C device.
 * @param fd File descriptor of the I2C device.
 */
void mpu6050_close(int fd);

#endif // MPU6050_DRIVER_H


