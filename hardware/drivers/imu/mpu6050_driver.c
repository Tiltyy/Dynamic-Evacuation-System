
/**
 * @file mpu6050_driver.c
 * @brief MPU6050 IMU Driver for Loongson System
 * @version 1.0
 * @date 2025-07-04
 *
 * This driver provides functions to initialize and read accelerometer,
 * gyroscope, and temperature data from the MPU6050 IMU via I2C.
 *
 * 中文注释：
 * 本驱动提供通过I2C接口初始化和读取MPU6050 IMU的加速度计、陀螺仪和温度数据的功能。
 */

 #include "mpu6050_driver.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <linux/i2c-dev.h>
 #include <errno.h>
 
 // Helper function to write a byte to an MPU6050 register
 // 辅助函数：向MPU6050寄存器写入一个字节
 static int mpu6050_write_byte(int fd, uint8_t reg, uint8_t value) {
     uint8_t buffer[2];
     buffer[0] = reg;
     buffer[1] = value;
     if (write(fd, buffer, 2) != 2) {
         perror("Failed to write MPU6050 register"); // 写入MPU6050寄存器失败
         return -1;
     }
     return 0;
 }
 
 // Helper function to read a byte from an MPU6050 register
 // 辅助函数：从MPU6050寄存器读取一个字节
 static int mpu6050_read_byte(int fd, uint8_t reg, uint8_t *value) {
     if (write(fd, &reg, 1) != 1) {
         perror("Failed to write MPU6050 register address"); // 写入MPU6050寄存器地址失败
         return -1;
     }
     if (read(fd, value, 1) != 1) {
         perror("Failed to read MPU6050 register"); // 读取MPU6050寄存器失败
         return -1;
     }
     return 0;
 }
 
 // Helper function to read multiple bytes from MPU6050 registers
 // 辅助函数：从MPU6050寄存器读取多个字节
 static int mpu6050_read_bytes(int fd, uint8_t reg, uint8_t *buffer, int len) {
     if (write(fd, &reg, 1) != 1) {
         perror("Failed to write MPU6050 register address for read"); // 写入MPU6050寄存器地址失败（用于读取）
         return -1;
     }
     if (read(fd, buffer, len) != len) {
         perror("Failed to read MPU6050 registers"); // 读取MPU6050寄存器失败
         return -1;
     }
     return 0;
 }
 
 /**
  * @brief Initializes the MPU6050 sensor.
  *        This function opens the I2C device, sets the slave address, and configures the MPU6050.
  * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-0").
  * @param address I2C address of the MPU6050 (MPU6050_ADDRESS_AD0_LOW or MPU6050_ADDRESS_AD0_HIGH).
  * @return File descriptor if successful, -1 otherwise.
  *
  * 中文注释：
  * 初始化MPU6050传感器。此函数打开I2C设备，设置从设备地址，并配置MPU6050。
  * @param i2c_dev_path I2C设备路径（例如，"/dev/i2c-0"）。
  * @param address MPU6050的I2C地址（MPU6050_ADDRESS_AD0_LOW或MPU6050_ADDRESS_AD0_HIGH）。
  * @return 成功返回文件描述符，否则返回-1。
  */
 int mpu6050_init(const char *i2c_dev_path, uint8_t address) {
     int fd;
     uint8_t who_am_i_val;
 
     // Open I2C device
     // 打开I2C设备
     fd = open(i2c_dev_path, O_RDWR);
     if (fd < 0) {
         fprintf(stderr, "Error: Could not open I2C device %s - %s\n", i2c_dev_path, strerror(errno));
         return -1;
     }
 
     // Set MPU6050 I2C slave address
     // 设置MPU6050的I2C从设备地址
     if (ioctl(fd, I2C_SLAVE, address) < 0) {
         fprintf(stderr, "Error: Could not set I2C address to 0x%02X - %s\n", address, strerror(errno));
         close(fd);
         return -1;
     }
 
     // Check WHO_AM_I register
     // 检查WHO_AM_I寄存器
     if (mpu6050_read_byte(fd, MPU6050_REG_WHO_AM_I, &who_am_i_val) < 0) {
         close(fd);
         return -1;
     }
     if (who_am_i_val != address) { // WHO_AM_I should return the slave address itself
         fprintf(stderr, "Error: MPU6050 WHO_AM_I mismatch. Expected 0x%02X, got 0x%02X\n", address, who_am_i_val); // 错误：MPU6050 WHO_AM_I不匹配。期望0x%02X，得到0x%02X
         close(fd);
         return -1;
     }
 
     // Wake up MPU6050 (PWR_MGMT_1 register)
     // 唤醒MPU6050（PWR_MGMT_1寄存器）
     if (mpu6050_write_byte(fd, MPU6050_REG_PWR_MGMT_1, 0x00) < 0) {
         close(fd);
         return -1;
     }
 
     // Configure accelerometer (ACCEL_CONFIG register)
     // 配置加速度计（ACCEL_CONFIG寄存器）
     if (mpu6050_write_byte(fd, MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_FS_2G) < 0) { // Set full scale range to +/- 2g
         close(fd);
         return -1;
     }
 
     // Configure gyroscope (GYRO_CONFIG register)
     // 配置陀螺仪（GYRO_CONFIG寄存器）
     if (mpu6050_write_byte(fd, MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_FS_250) < 0) { // Set full scale range to +/- 250 deg/s
         close(fd);
         return -1;
     }
 
     printf("MPU6050 sensor initialized successfully at address 0x%02X.\n", address); // MPU6050传感器在地址0x%02X处初始化成功。
     return fd;
 }
 
 /**
  * @brief Reads all sensor data (accelerometer, gyroscope, temperature) from MPU6050.
  * @param fd File descriptor of the I2C device.
  * @param data Pointer to MPU6050_Data_t structure to store the read data.
  * @return 0 if successful, -1 otherwise.
  *
  * 中文注释：
  * 从MPU6050读取所有传感器数据（加速度计、陀螺仪、温度）。
  * @param fd I2C设备的文件描述符。
  * @param data 指向MPU6050_Data_t结构体的指针，用于存储读取的数据。
  * @return 成功返回0，否则返回-1。
  */
 int mpu6050_read_data(int fd, MPU6050_Data_t *data) {
     uint8_t raw_data[14]; // 6 bytes Accel, 2 bytes Temp, 6 bytes Gyro
 
     // Read 14 bytes starting from ACCEL_XOUT_H
     // 从ACCEL_XOUT_H开始读取14个字节
     if (mpu6050_read_bytes(fd, MPU6050_REG_ACCEL_XOUT_H, raw_data, 14) < 0) {
         return -1;
     }
 
     // Accelerometer data
     // 加速度计数据
     data->accel_x = (raw_data[0] << 8) | raw_data[1];
     data->accel_y = (raw_data[2] << 8) | raw_data[3];
     data->accel_z = (raw_data[4] << 8) | raw_data[5];
 
     // Temperature data
     // 温度数据
     data->temperature = (raw_data[6] << 8) | raw_data[7];
 
     // Gyroscope data
     // 陀螺仪数据
     data->gyro_x = (raw_data[8] << 8) | raw_data[9];
     data->gyro_y = (raw_data[10] << 8) | raw_data[11];
     data->gyro_z = (raw_data[12] << 8) | raw_data[13];
 
     return 0;
 }
 
 /**
  * @brief Closes the MPU6050 I2C device.
  * @param fd File descriptor of the I2C device.
  *
  * 中文注释：
  * 关闭MPU6050 I2C设备。
  * @param fd I2C设备的文件描述符。
  */
 void mpu6050_close(int fd) {
     if (fd >= 0) {
         close(fd);
         printf("MPU6050 I2C device closed.\n"); // MPU6050 I2C设备已关闭。
     }
 }
 
 // Example usage (for testing purposes)
 // 示例用法（仅用于测试）
 #ifdef MPU6050_TEST
 int main() {
     int mpu_fd;
     MPU6050_Data_t sensor_data;
     const char *i2c_dev = "/dev/i2c-0"; // Adjust this to your actual I2C device path
                                         // 根据您的实际I2C设备路径进行调整
     uint8_t mpu_address = MPU6050_ADDRESS_AD0_LOW; // Set to 0x68 if AD0 is GND, 0x69 if AD0 is VCC
                                                   // 如果AD0接地设置为0x68，如果AD0接VCC设置为0x69
 
     // Initialize MPU6050
     // 初始化MPU6050
     mpu_fd = mpu6050_init(i2c_dev, mpu_address);
     if (mpu_fd < 0) {
         return -1;
     }
 
     printf("Reading MPU6050 data...\n"); // 正在读取MPU6050数据...
     for (int i = 0; i < 10; i++) {
         if (mpu6050_read_data(mpu_fd, &sensor_data) == 0) {
             printf("Accel: X=%d, Y=%d, Z=%d | Gyro: X=%d, Y=%d, Z=%d | Temp: %d\n",
                    sensor_data.accel_x, sensor_data.accel_y, sensor_data.accel_z,
                    sensor_data.gyro_x, sensor_data.gyro_y, sensor_data.gyro_z,
                    sensor_data.temperature);
         } else {
             fprintf(stderr, "Failed to read MPU6050 data.\n"); // 读取MPU6050数据失败。
         }
         usleep(100000); // Wait for 100ms
     }
 
     // Close MPU6050 device
     // 关闭MPU6050设备
     mpu6050_close(mpu_fd);
 
     return 0;
 }
 #endif // MPU6050_TEST
 
 
 