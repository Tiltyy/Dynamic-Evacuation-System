
/**
 * @file sgp30_driver.c
 * @brief SGP30 Gas Sensor Driver for Loongson System
 * @version 1.0
 * @date 2025-07-04
 *
 * This driver provides functions to initialize, read TVOC and eCO2 values
 * from the Sensirion SGP30 gas sensor via I2C.
 *
 * 中文注释：
 * 本驱动提供通过I2C接口初始化、读取Sensirion SGP30气体传感器TVOC和eCO2值的功能。
 */

 #include "sgp30_driver.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <linux/i2c-dev.h>
 #include <errno.h>
 
 // Helper function to calculate CRC8 for SGP30 data
 // 辅助函数：计算SGP30数据的CRC8校验码
 static uint8_t sgp30_crc8(uint8_t *data, int len) {
     uint8_t crc = 0xFF;
     uint8_t poly = 0x31; // CRC-8 polynomial
     int i, j;
 
     for (i = 0; i < len; i++) {
         crc ^= data[i];
         for (j = 0; j < 8; j++) {
             if (crc & 0x80) {
                 crc = (crc << 1) ^ poly;
             } else {
                 crc <<= 1;
             }
         }
     }
     return crc;
 }
 
 // Helper function to write a command to SGP30
 // 辅助函数：向SGP30写入命令
 static int sgp30_write_command(int fd, uint16_t command) {
     uint8_t buffer[2];
     buffer[0] = (command >> 8) & 0xFF; // MSB
     buffer[1] = command & 0xFF;        // LSB
 
     if (write(fd, buffer, 2) != 2) {
         perror("Failed to write SGP30 command"); // 写入SGP30命令失败
         return -1;
     }
     usleep(10000); // SGP30 needs some time to process commands (min 0.5ms)
     return 0;
 }
 
 // Helper function to read data from SGP30
 // 辅助函数：从SGP30读取数据
 static int sgp30_read_data(int fd, uint8_t *buffer, int len) {
     int bytes_read = read(fd, buffer, len);
     if (bytes_read != len) {
         perror("Failed to read SGP30 data"); // 读取SGP30数据失败
         return -1;
     }
     return 0;
 }
 
 /**
  * @brief Initializes the SGP30 sensor.
  *        This function opens the I2C device and sends the initialization command.
  * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-0").
  * @return File descriptor if successful, -1 otherwise.
  *
  * 中文注释：
  * 初始化SGP30传感器。此函数打开I2C设备并发送初始化命令。
  * @param i2c_dev_path I2C设备路径（例如，"/dev/i2c-0"）。
  * @return 成功返回文件描述符，否则返回-1。
  */
 int sgp30_init(const char *i2c_dev_path) {
     int fd;
 
     // Open I2C device
     // 打开I2C设备
     fd = open(i2c_dev_path, O_RDWR);
     if (fd < 0) {
         fprintf(stderr, "Error: Could not open I2C device %s - %s\n", i2c_dev_path, strerror(errno));
         return -1;
     }
 
     // Set SGP30 I2C slave address
     // 设置SGP30的I2C从设备地址
     if (ioctl(fd, I2C_SLAVE, SGP30_I2C_ADDRESS) < 0) {
         fprintf(stderr, "Error: Could not set I2C address to 0x%02X - %s\n", SGP30_I2C_ADDRESS, strerror(errno));
         close(fd);
         return -1;
     }
 
     // Send Init_Air_Quality command
     // 发送Init_Air_Quality命令
     if (sgp30_write_command(fd, SGP30_INIT_AIR_QUALITY) < 0) {
         close(fd);
         return -1;
     }
     usleep(10000); // Wait for initialization (min 10ms)
     printf("SGP30 sensor initialized successfully.\n"); // SGP30传感器初始化成功。
     return fd;
 }
 
 /**
  * @brief Reads TVOC and eCO2 values from the SGP30 sensor.
  *        This function sends the measure command and reads the data.
  * @param fd File descriptor of the I2C device.
  * @param tvoc Pointer to store the TVOC value.
  * @param eco2 Pointer to store the eCO2 value.
  * @return 0 if successful, -1 otherwise.
  *
  * 中文注释：
  * 从SGP30传感器读取TVOC和eCO2值。此函数发送测量命令并读取数据。
  * @param fd I2C设备的文件描述符。
  * @param tvoc 用于存储TVOC值的指针。
  * @param eco2 用于存储eCO2值的指针。
  * @return 成功返回0，否则返回-1。
  */
 int sgp30_read_air_quality(int fd, uint16_t *tvoc, uint16_t *eco2) {
     uint8_t read_buffer[6]; // 2 bytes TVOC, 1 byte CRC, 2 bytes eCO2, 1 byte CRC
 
     // Send Measure_Air_Quality command
     // 发送Measure_Air_Quality命令
     if (sgp30_write_command(fd, SGP30_MEASURE_AIR_QUALITY) < 0) {
         return -1;
     }
     usleep(12000); // SGP30 needs 12ms to perform measurement
                    // SGP30需要12毫秒来执行测量
 
     // Read 6 bytes: TVOC (2 bytes) + CRC (1 byte) + eCO2 (2 bytes) + CRC (1 byte)
     // 读取6个字节：TVOC（2字节）+ CRC（1字节）+ eCO2（2字节）+ CRC（1字节）
     if (sgp30_read_data(fd, read_buffer, 6) < 0) {
         return -1;
     }
 
     // Verify CRC for TVOC
     // 验证TVOC的CRC
     if (sgp30_crc8(read_buffer, 2) != read_buffer[2]) {
         fprintf(stderr, "Error: SGP30 TVOC CRC mismatch.\n"); // 错误：SGP30 TVOC CRC不匹配。
         return -1;
     }
 
     // Verify CRC for eCO2
     // 验证eCO2的CRC
     if (sgp30_crc8(read_buffer + 3, 2) != read_buffer[5]) {
         fprintf(stderr, "Error: SGP30 eCO2 CRC mismatch.\n"); // 错误：SGP30 eCO2 CRC不匹配。
         return -1;
     }
 
     // Extract TVOC and eCO2 values
     // 提取TVOC和eCO2值
     *tvoc = (read_buffer[0] << 8) | read_buffer[1];
     *eco2 = (read_buffer[3] << 8) | read_buffer[4];
 
     return 0;
 }
 
 /**
  * @brief Closes the SGP30 I2C device.
  * @param fd File descriptor of the I2C device.
  *
  * 中文注释：
  * 关闭SGP30 I2C设备。
  * @param fd I2C设备的文件描述符。
  */
 void sgp30_close(int fd) {
     if (fd >= 0) {
         close(fd);
         printf("SGP30 I2C device closed.\n"); // SGP30 I2C设备已关闭。
     }
 }
 
 // Example usage (for testing purposes)
 // 示例用法（仅用于测试）
 #ifdef SGP30_TEST
 int main() {
     int sgp30_fd;
     uint16_t tvoc_val, eco2_val;
     const char *i2c_dev = "/dev/i2c-0"; // Adjust this to your actual I2C device path
                                         // 根据您的实际I2C设备路径进行调整
 
     // Initialize SGP30
     // 初始化SGP30
     sgp30_fd = sgp30_init(i2c_dev);
     if (sgp30_fd < 0) {
         return -1;
     }
 
     // First measurement will return 400ppm eCO2 and 0ppb TVOC
     // 第一次测量将返回400ppm eCO2和0ppb TVOC
     printf("Performing initial SGP30 measurement...\n"); // 正在执行SGP30初始测量...
     if (sgp30_read_air_quality(sgp30_fd, &tvoc_val, &eco2_val) == 0) {
         printf("Initial SGP30: TVOC = %u ppb, eCO2 = %u ppm\n", tvoc_val, eco2_val);
     } else {
         fprintf(stderr, "Failed to read initial SGP30 data.\n"); // 读取SGP30初始数据失败。
     }
     sleep(1); // Wait for 1 second before next measurement
 
     // Read air quality every second
     // 每秒读取空气质量
     for (int i = 0; i < 10; i++) {
         if (sgp30_read_air_quality(sgp30_fd, &tvoc_val, &eco2_val) == 0) {
             printf("SGP30: TVOC = %u ppb, eCO2 = %u ppm\n", tvoc_val, eco2_val);
         } else {
             fprintf(stderr, "Failed to read SGP30 data.\n"); // 读取SGP30数据失败。
         }
         sleep(1);
     }
 
     // Close SGP30 device
     // 关闭SGP30设备
     sgp30_close(sgp30_fd);
 
     return 0;
 }
 #endif // SGP30_TEST
 
 
 