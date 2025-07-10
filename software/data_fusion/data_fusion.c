/**
 * @file data_fusion.c
 * @brief Data Fusion Module for Dynamic Emergency Evacuation System
 * @version 1.0
 * @date 2025-07-04
 *
 * This module is responsible for fusing data from various sensors
 * (SGP30, MQ-2, MPU6050) to create a comprehensive environmental and motion model.
 *
 * 中文注释：
 * 本模块负责融合来自各种传感器（SGP30、MQ-2、MPU6050）的数据，以创建全面的环境和运动模型。
 */

 #include "data_fusion.h"
 #include <stdio.h>
 #include <math.h>
 #include <string.h>
 #include <time.h>
 #include <errno.h> // For strerror
 
 // Calibration values for MQ-2 sensor (example values, need to be calibrated)
 // MQ-2传感器的校准值（示例值，需要校准）
 #define MQ2_RL_VALUE 10.0 // Resistance of the load resistor in kOhms
 #define MQ2_RO_CLEAN_AIR_FACTOR 9.83 // Ro in clean air (from datasheet or calibration)
 
 // MPU6050 conversion factors
 // MPU6050转换系数
 #define ACCEL_LSB_PER_G 16384.0 // For +/- 2g range
 #define GYRO_LSB_PER_DPS 131.0 // For +/- 250 deg/s range
 
 // Kalman filter parameters for orientation (example values, need tuning)
 // 用于姿态的卡尔曼滤波参数（示例值，需要调整）
 static float Q_angle = 0.001; // Process noise variance for the angle
 static float Q_bias = 0.003;  // Process noise variance for the bias
 static float R_measure = 0.03; // Measurement noise variance
 
 static float angle = 0.0; // Kalman filter estimated angle
 static float bias = 0.0;  // Kalman filter estimated bias
 static float P[2][2] = {{0.0, 0.0}, {0.0, 0.0}}; // Error covariance matrix
 
 /**
  * @brief Initializes the data fusion module.
  * @return 0 if successful, -1 otherwise.
  *
  * 中文注释：
  * 初始化数据融合模块。
  * @return 成功返回0，否则返回-1。
  */
 int data_fusion_init(void) {
     printf("Data fusion module initialized.\n"); // 数据融合模块已初始化。
     return 0;
 }
 
 /**
  * @brief Converts MQ-2 raw ADC value to resistance Rs.
  * @param raw_adc Raw ADC value from ADS1115.
  * @return Resistance Rs in kOhms.
  *
  * 中文注释：
  * 将MQ-2原始ADC值转换为电阻Rs。
  * @param raw_adc 来自ADS1115的原始ADC值。
  * @return 电阻Rs，单位kOhms。
  */
 static float mq2_get_rs(int16_t raw_adc) {
     float voltage = (float)raw_adc * 2.048 / 32767.0; // Assuming +/-2.048V range for ADS1115
     if (voltage == 0) return 0; // Avoid division by zero
     return (MQ2_RL_VALUE * (2.048 - voltage)) / voltage;
 }
 
 /**
  * @brief Converts MQ-2 resistance Rs to concentration (example for LPG).
  *        This function uses a simplified power law model. Real calibration is needed.
  * @param rs Resistance Rs in kOhms.
  * @return Concentration in ppm.
  *
  * 中文注释：
  * 将MQ-2电阻Rs转换为浓度（LPG示例）。
  * 此函数使用简化的幂律模型。需要实际校准。
  * @param rs 电阻Rs，单位kOhms。
  * @return 浓度，单位ppm。
  */
 static float mq2_get_concentration(float rs) {
     // This is a simplified example. Real MQ-2 calibration involves a log-log plot
     // of Rs/Ro vs. concentration for specific gases.
     // 这是一个简化示例。真实的MQ-2校准涉及特定气体的Rs/Ro与浓度之间的对数-对数图。
     float rs_ro = rs / MQ2_RO_CLEAN_AIR_FACTOR;
     // Example: For LPG, if Rs/Ro = 0.5, concentration might be around 200ppm
     // You need to get the actual curve from the datasheet or calibrate it yourself.
     // 示例：对于LPG，如果Rs/Ro = 0.5，浓度可能在200ppm左右
     // 您需要从数据手册中获取实际曲线或自行校准。
     if (rs_ro == 0) return 0;
     return 100.0 * pow(rs_ro, -2.5); // Example power law, adjust as needed
 }
 
 /**
  * @brief Fuses environmental sensor data (SGP30, MQ-2).
  * @param sgp30_tvoc TVOC value from SGP30.
  * @param sgp30_eco2 eCO2 value from SGP30.
  * @param mq2_raw_adc Raw ADC value from MQ-2 via ADS1115.
  * @param env_data Pointer to EnvironmentalData_t structure to store fused data.
  * @return 0 if successful, -1 otherwise.
  *
  * 中文注释：
  * 融合环境传感器数据（SGP30、MQ-2）。
  * @param sgp30_tvoc 来自SGP30的TVOC值。
  * @param sgp30_eco2 来自SGP30的eCO2值。
  * @param mq2_raw_adc 通过ADS1115从MQ-2获得的原始ADC值。
  * @param env_data 指向EnvironmentalData_t结构体的指针，用于存储融合数据。
  * @return 成功返回0，否则返回-1。
  */
 int fuse_environmental_data(uint16_t sgp30_tvoc, uint16_t sgp30_eco2, int16_t mq2_raw_adc, EnvironmentalData_t *env_data) {
     env_data->tvoc_ppb = sgp30_tvoc;
     env_data->eco2_ppm = sgp30_eco2;
 
     env_data->mq2_voltage = (float)mq2_raw_adc * 2.048 / 32767.0; // Convert to voltage
     float rs = mq2_get_rs(mq2_raw_adc);
     env_data->mq2_concentration = mq2_get_concentration(rs);
 
     return 0;
 }
 
 /**
  * @brief Fuses motion sensor data (MPU6050) using a simplified Kalman filter for orientation.
  * @param mpu6050_data Raw data from MPU6050.
  * @param motion_data Pointer to MotionData_t structure to store fused data.
  * @return 0 if successful, -1 otherwise.
  *
  * 中文注释：
  * 使用简化的卡尔曼滤波器融合运动传感器数据（MPU6050）以获取姿态。
  * @param mpu6050_data 来自MPU6050的原始数据。
  * @param motion_data 指向MotionData_t结构体的指针，用于存储融合数据。
  * @return 成功返回0，否则返回-1。
  */
 int fuse_motion_data(MPU6050_Data_t mpu6050_data, MotionData_t *motion_data) {
     // Convert raw accelerometer and gyroscope values to physical units
     // 将原始加速度计和陀螺仪值转换为物理单位
     motion_data->accel_x_g = (float)mpu6050_data.accel_x / ACCEL_LSB_PER_G;
     motion_data->accel_y_g = (float)mpu6050_data.accel_y / ACCEL_LSB_PER_G;
     motion_data->accel_z_g = (float)mpu6050_data.accel_z / ACCEL_LSB_PER_G;
 
     motion_data->gyro_x_dps = (float)mpu6050_data.gyro_x / GYRO_LSB_PER_DPS;
     motion_data->gyro_y_dps = (float)mpu6050_data.gyro_y / GYRO_LSB_PER_DPS;
     motion_data->gyro_z_dps = (float)mpu6050_data.gyro_z / GYRO_LSB_PER_DPS;
 
     // Simplified Kalman filter for pitch/roll (example for one axis)
     // 简化版卡尔曼滤波器用于俯仰/横滚（单轴示例）
     // This is a very basic implementation. A full IMU fusion would involve quaternions or DCM.
     // 这是一个非常基本的实现。完整的IMU融合将涉及四元数或DCM。
     float dt = 0.1; // Time step in seconds (adjust based on your sensor reading frequency)
                     // 时间步长（秒），根据传感器读取频率调整
 
     // Predict
     // 预测
     float gyro_rate = motion_data->gyro_x_dps; // Using X-axis gyro for pitch example
     angle += dt * (gyro_rate - bias);
     P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
     P[0][1] -= dt * P[1][1];
     P[1][0] -= dt * P[1][1];
     P[1][1] += Q_bias * dt;
 
     // Update
     // 更新
     float accel_angle = atan2(motion_data->accel_y_g, sqrt(motion_data->accel_x_g * motion_data->accel_x_g + motion_data->accel_z_g * motion_data->accel_z_g)) * 180 / M_PI;
     float y = accel_angle - angle;
     float S = P[0][0] + R_measure;
     float K[2];
     K[0] = P[0][0] / S;
     K[1] = P[1][0] / S;
 
     angle += K[0] * y;
     bias += K[1] * y;
     float P00 = P[0][0];
     float P01 = P[0][1];
     P[0][0] -= K[0] * P00;
     P[0][1] -= K[0] * P01;
     P[1][0] -= K[1] * P00;
     P[1][1] -= K[1] * P01;
 
     motion_data->pitch = angle; // Example: pitch is the fused angle
     motion_data->roll = 0.0; // Placeholder
     motion_data->yaw = 0.0;  // Placeholder
 
     return 0;
 }
 
 // Example usage (for testing purposes)
 // 示例用法（仅用于测试）
 #ifdef DATA_FUSION_TEST
 #include "../../hardware/drivers/gas_sensor/sgp30_driver.h"
 #include "../../hardware/drivers/gas_sensor/ads1115_driver.h"
 #include "../../hardware/drivers/imu/mpu6050_driver.h"
 #include <unistd.h>
 
 int main() {
     // --- Sensor Initialization (mock or real) ---
     // 传感器初始化（模拟或真实）
     int sgp30_fd = -1; // Mock or real FD
     int ads_fd = -1;   // Mock or real FD
     int mpu_fd = -1;   // Mock or real FD
 
     // For real testing, uncomment and adjust paths/addresses
     // 对于真实测试，请取消注释并调整路径/地址
     // sgp30_fd = sgp30_init("/dev/i2c-0");
     // ads_fd = ads1115_init("/dev/i2c-1", ADS1115_ADDRESS_GND);
     // mpu_fd = mpu6050_init("/dev/i2c-0", MPU6050_ADDRESS_AD0_LOW);
 
     if (sgp30_fd < 0 || ads_fd < 0 || mpu_fd < 0) {
         fprintf(stderr, "Warning: Sensor initialization failed or mocked. Using dummy data.\n"); // 警告：传感器初始化失败或模拟。正在使用虚拟数据。
     }
 
     data_fusion_init();
 
     EnvironmentalData_t env_data;
     MotionData_t motion_data;
 
     // Dummy data for testing
     uint16_t dummy_tvoc = 100;
     uint16_t dummy_eco2 = 500;
     int16_t dummy_mq2_adc = 15000;
     MPU6050_Data_t dummy_mpu_data = {
         .accel_x = 1000, .accel_y = 2000, .accel_z = 16000,
         .gyro_x = 50, .gyro_y = 20, .gyro_z = 10
     };
 
     printf("Performing data fusion...\n");
     for (int i = 0; i < 5; i++) {
         fuse_environmental_data(dummy_tvoc, dummy_eco2, dummy_mq2_adc, &env_data);
         fuse_motion_data(dummy_mpu_data, &motion_data);
 
         printf("\nFused Environmental Data:\n");
         printf("  TVOC: %u ppb\n", env_data.tvoc_ppb);
         printf("  eCO2: %u ppm\n", env_data.eco2_ppm);
         printf("  MQ2 Voltage: %.4f V\n", env_data.mq2_voltage);
         printf("  MQ2 Concentration: %.2f ppm\n", env_data.mq2_concentration);
 
         printf("Fused Motion Data:\n");
         printf("  Accel (X,Y,Z): %.2f, %.2f, %.2f g\n", motion_data.accel_x_g, motion_data.accel_y_g, motion_data.accel_z_g);
         printf("  Gyro (X,Y,Z): %.2f, %.2f, %.2f dps\n", motion_data.gyro_x_dps, motion_data.gyro_y_dps, motion_data.gyro_z_dps);
         printf("  Pitch: %.2f deg\n", motion_data.pitch);
 
         sleep(1);
     }
 
     // Close real FDs if opened
     // if (sgp30_fd >= 0) sgp30_close(sgp30_fd);
     // if (ads_fd >= 0) ads1115_close(ads_fd);
     // if (mpu_fd >= 0) mpu6050_close(mpu_fd);
 
     return 0;
 }
 #endif // DATA_FUSION_TEST
 