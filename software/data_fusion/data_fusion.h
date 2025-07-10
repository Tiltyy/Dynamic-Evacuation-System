#ifndef DATA_FUSION_H
#define DATA_FUSION_H

#include <stdint.h>
#include "../../hardware/drivers/imu/mpu6050_driver.h"

// Structure to hold fused environmental data
// 用于存储融合环境数据的结构体
typedef struct {
    uint16_t tvoc_ppb;
    uint16_t eco2_ppm;
    float mq2_voltage;
    float mq2_concentration; // e.g., ppm
} EnvironmentalData_t;

// Structure to hold fused motion data
// 用于存储融合运动数据的结构体
typedef struct {
    float accel_x_g, accel_y_g, accel_z_g; // Acceleration in G's
    float gyro_x_dps, gyro_y_dps, gyro_z_dps; // Angular velocity in degrees per second
    float roll, pitch, yaw; // Orientation in degrees
} MotionData_t;

// Function prototypes
// 函数原型
/**
 * @brief Initializes the data fusion module.
 * @return 0 if successful, -1 otherwise.
 */
int data_fusion_init(void);

/**
 * @brief Fuses environmental sensor data (SGP30, MQ-2).
 * @param sgp30_tvoc TVOC value from SGP30.
 * @param sgp30_eco2 eCO2 value from SGP30.
 * @param mq2_raw_adc Raw ADC value from MQ-2 via ADS1115.
 * @param env_data Pointer to EnvironmentalData_t structure to store fused data.
 * @return 0 if successful, -1 otherwise.
 */
int fuse_environmental_data(uint16_t sgp30_tvoc, uint16_t sgp30_eco2, int16_t mq2_raw_adc, EnvironmentalData_t *env_data);

/**
 * @brief Fuses motion sensor data (MPU6050).
 * @param mpu6050_data Raw data from MPU6050.
 * @param motion_data Pointer to MotionData_t structure to store fused data.
 * @return 0 if successful, -1 otherwise.
 */
int fuse_motion_data(MPU6050_Data_t mpu6050_data, MotionData_t *motion_data);

#endif // DATA_FUSION_H


