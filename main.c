/**
 * @file main.c
 * @brief Main application for Dynamic Emergency Evacuation System
 * @version 1.0
 * @date 2025-07-04
 *
 * This file integrates all modules (hardware drivers, data fusion, path planning, UI)
 * to provide a complete system for dynamic emergency evacuation guidance.
 *
 * 中文注释：
 * 本文件集成了所有模块（硬件驱动、数据融合、路径规划、UI），
 * 以提供一个完整的动态应急疏散引导系统。
 */

 #include <stdio.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h> // For time_t
 
 // Hardware Drivers
 #include "hardware/drivers/rfid/rfid_driver.h"
 #include "hardware/drivers/gas_sensor/sgp30_driver.h"
 #include "hardware/drivers/gas_sensor/ads1115_driver.h"
 #include "hardware/drivers/imu/mpu6050_driver.h"
 // #include "hardware/drivers/lcd12864/lcd12864.h" // Replaced by SSD1306 OLED
 #include "hardware/drivers/ssd1306_linux/ssd1306.h"
 
 // Software Modules
 #include "software/data_fusion/data_fusion.h"
 #include "software/path_planning/path_planning.h"
 
 // UI Module
 #include "demonstration/ui/ui_module.h"
 
 // Define GPIO pins for buzzer and LCD/OLED control
 #define BUZZER_GPIO_PIN 2 // Example GPIO pin for buzzer
 
 // Define I2C device path for MPU6050
 #define MPU6050_I2C_PATH "/dev/i2c-0" // Example I2C bus for MPU6050
#define MPU6050_I2C_ADDR MPU6050_ADDRESS
 
 // Define SPI path for LCD12864 (if still used, otherwise remove)
 // #define LCD12864_SPI_PATH "/dev/spidev0.0" // LCD12864 and RC522 on SPI-0
 
 void system_init(int *rfid_fd_ptr, int *sgp30_fd_ptr, int *ads_fd_ptr, int *mpu_fd_ptr);
void system_cleanup(int rfid_fd, int sgp30_fd, int ads_fd, int mpu_fd);

int main()
{
    printf("Dynamic Emergency Evacuation System Starting...\n");

    // Declare file descriptors locally
    int rfid_fd = -1;
    int sgp30_fd = -1;
    int ads_fd = -1;
    int mpu_fd = -1;

    // Global variables for sensor data
    MPU6050_Data_t mpu_raw_data;
    MotionData_t motion_data;
    EnvironmentalData_t env_data;

    path_t evacuation_path;

    // Initialize system components
    system_init(&rfid_fd, &sgp30_fd, &ads_fd, &mpu_fd);
 
     // Main loop
     while (1)
     {
         // 1. Read sensor data
         // MPU6050
         if (mpu_fd >= 0 && mpu6050_read_data(mpu_fd, &mpu_raw_data) != 0)
         {
             fprintf(stderr, "Error: Failed to read MPU6050 data\n");
         }
 
         // SGP30 (Gas Sensor)
         if (sgp30_fd >= 0 && sgp30_read_air_quality(sgp30_fd, &env_data.tvoc_ppb, &env_data.eco2_ppm) != 0)
         {
             fprintf(stderr, "Error: Failed to read SGP30 data\n");
         }
 
         // ADS1115 (Analog to Digital Converter for other sensors like MQ-2)
         // Assuming MQ-2 is connected to A0 of ADS1115
         if (ads_fd >= 0 && ads1115_read_adc_channel(ads_fd, 0) != -1)
         {
             env_data.mq2_voltage = (float)ads1115_read_adc_channel(ads_fd, 0) * (2.048 / 32767.0); // Assuming PGA +/-2.048V
             // Placeholder for MQ-2 concentration calculation, needs calibration
             env_data.mq2_concentration = env_data.mq2_voltage * 100.0; // Example conversion
         } else if (ads_fd >= 0) {
             fprintf(stderr, "Error: Failed to read ADS1115 data\n");
         }
 
         // 2. Data Fusion
         fuse_motion_data(mpu_raw_data, &motion_data);
         fuse_environmental_data(env_data.tvoc_ppb, env_data.eco2_ppm, (int16_t)(env_data.mq2_voltage / (2.048 / 32767.0)), &env_data); // Pass raw ADC value for fusion
 
         // 3. Path Planning (Example: find a safe path based on current location and environmental data)
         // This is a placeholder. Actual path planning would involve more complex logic.
         // For demonstration, let\'s assume a fixed start and end point for evacuation.
         // You would need to define your map and nodes appropriately.
         // path_len = find_safe_path(MAP_WIDTH, MAP_HEIGHT, evacuation_path, MAP_WIDTH * MAP_HEIGHT);
         // For now, let\'s just update UI with dummy path data or sensor data
 
         // 4. UI Update
         ui_update(&env_data, &motion_data, &evacuation_path);
 
         // 5. Check for alerts (e.g., high gas levels)
         if (env_data.mq2_concentration > 50.0 || env_data.eco2_ppm > 1000) // Example thresholds
         {
             ui_trigger_alert(500); // Trigger alert for 500ms
         }
 
         usleep(500000); // Update every 500ms
     }
 
     // Cleanup system components (this part will not be reached in current infinite loop)
    system_cleanup(rfid_fd, sgp30_fd, ads_fd, mpu_fd);
 
     return 0;
 }
 
void system_init(int *rfid_fd_ptr, int *sgp30_fd_ptr, int *ads_fd_ptr, int *mpu_fd_ptr) {
     // Initialize RFID
     *rfid_fd_ptr = rfid_init("/dev/ttyUSB0"); // Assuming RFID is on /dev/ttyUSB0
     if (*rfid_fd_ptr < 0)
     {
         fprintf(stderr, "Error: Failed to initialize RFID reader\n");
         // Handle error, maybe exit or retry
     }
 
     // Initialize SGP30
     *sgp30_fd_ptr = sgp30_init("/dev/i2c-0"); // Assuming SGP30 is on /dev/i2c-0
     if (*sgp30_fd_ptr < 0)
     {
         fprintf(stderr, "Error: Failed to initialize SGP30 sensor\n");
     }
 
     // Initialize ADS1115
     *ads_fd_ptr = ads1115_init("/dev/i2c-1", ADS1115_ADDRESS_GND); // Assuming ADS1115 is on /dev/i2c-1, ADDR to GND
     if (*ads_fd_ptr < 0)
     {
         fprintf(stderr, "Error: Failed to initialize ADS1115\n");
     }
 
     // Initialize MPU6050
     *mpu_fd_ptr = mpu6050_init(MPU6050_I2C_PATH, MPU6050_ADDRESS_AD0_LOW);
 
     // Initialize UI module (OLED)
     if (ui_init(0, BUZZER_GPIO_PIN) != 0) // 0 for /dev/i2c-0, BUZZER_GPIO_PIN for buzzer control
     {
         fprintf(stderr, "Failed to initialize UI module.\n");
         // return -1;
     }
 
     // Initialize data fusion module
     data_fusion_init();
 
     // Initialize path planning module (if needed, e.g., load map data)
     path_planning_init();
 }
 
void system_cleanup(int rfid_fd, int sgp30_fd, int ads_fd, int mpu_fd) {
     // Cleanup RFID
     rfid_close(rfid_fd);
 
     // Cleanup SGP30
     sgp30_close(sgp30_fd);
 
     // Cleanup ADS1115
     ads1115_close(ads_fd);
 
     // Cleanup MPU6050
     mpu6050_close(mpu_fd);
 
     // Cleanup UI module
     ui_cleanup();
 

 
     // Cleanup path planning module
     path_planning_cleanup();
 }
 