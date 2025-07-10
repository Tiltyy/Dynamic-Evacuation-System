#include "ads1115_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>

// ADS1115 I2C addresses
#define ADS1115_ADDRESS_GND 0x48
#define ADS1115_ADDRESS_VCC 0x49
#define ADS1115_ADDRESS_SDA 0x4A
#define ADS1115_ADDRESS_SCL 0x4B

// ADS1115 Register Pointers
#define ADS1115_REG_POINTER_CONVERT 0x00
#define ADS1115_REG_POINTER_CONFIG  0x01
#define ADS1115_REG_POINTER_LOWTHRESH 0x02
#define ADS1115_REG_POINTER_HITHRESH  0x03

// ADS1115 Configuration Register Bits
#define ADS1115_CONFIG_OS_SINGLE    0x8000 // OS: Start a single conversion
#define ADS1115_CONFIG_MUX_P0_N1    0x4000 // MUX: P0 = AINP, N1 = AINN
#define ADS1115_CONFIG_MUX_P0_GND   0x4000 // MUX: P0 = AINP, GND = AINN (for single-ended)
#define ADS1115_CONFIG_MUX_P1_GND   0x5000 // MUX: P1 = AINP, GND = AINN
#define ADS1115_CONFIG_MUX_P2_GND   0x6000 // MUX: P2 = AINP, GND = AINN
#define ADS1115_CONFIG_MUX_P3_GND   0x7000 // MUX: P3 = AINP, GND = AINN

#define ADS1115_CONFIG_PGA_6_144V   0x0000 // PGA: +/-6.144V
#define ADS1115_CONFIG_PGA_4_096V   0x0200 // PGA: +/-4.096V
#define ADS1115_CONFIG_PGA_2_048V   0x0400 // PGA: +/-2.048V (default)
#define ADS1115_CONFIG_PGA_1_024V   0x0600 // PGA: +/-1.024V
#define ADS1115_CONFIG_PGA_0_512V   0x0800 // PGA: +/-0.512V
#define ADS1115_CONFIG_PGA_0_256V   0x0A00 // PGA: +/-0.256V

#define ADS1115_CONFIG_MODE_CONTINUOUS 0x0000 // MODE: Continuous conversion mode
#define ADS1115_CONFIG_MODE_SINGLE     0x0100 // MODE: Single-shot mode (default)

#define ADS1115_CONFIG_DR_8SPS      0x0000 // DR: 8 SPS
#define ADS1115_CONFIG_DR_16SPS     0x0020 // DR: 16 SPS
#define ADS1115_CONFIG_DR_32SPS     0x0040 // DR: 32 SPS
#define ADS1115_CONFIG_DR_64SPS     0x0060 // DR: 64 SPS
#define ADS1115_CONFIG_DR_128SPS    0x0080 // DR: 128 SPS (default)
#define ADS1115_CONFIG_DR_250SPS    0x00A0 // DR: 250 SPS
#define ADS1115_CONFIG_DR_475SPS    0x00C0 // DR: 475 SPS
#define ADS1115_CONFIG_DR_860SPS    0x00E0 // DR: 860 SPS

#define ADS1115_CONFIG_COMP_MODE_TRAD 0x0000 // COMP_MODE: Traditional comparator
#define ADS1115_CONFIG_COMP_MODE_WINDOW 0x0010 // COMP_MODE: Window comparator

#define ADS1115_CONFIG_COMP_POL_LOW 0x0000 // COMP_POL: Active low
#define ADS1115_CONFIG_COMP_POL_HIGH 0x0008 // COMP_POL: Active high

#define ADS1115_CONFIG_COMP_LAT_NONLATCH 0x0000 // COMP_LAT: Non-latching comparator
#define ADS1115_CONFIG_COMP_LAT_LATCH  0x0004 // COMP_LAT: Latching comparator

#define ADS1115_CONFIG_COMP_QUE_1CONV 0x0000 // COMP_QUE: Assert after 1 conversion
#define ADS1115_CONFIG_COMP_QUE_2CONV 0x0001 // COMP_QUE: Assert after 2 conversions
#define ADS1115_CONFIG_COMP_QUE_4CONV 0x0002 // COMP_QUE: Assert after 4 conversions
#define ADS1115_CONFIG_COMP_QUE_DISABLE 0x0003 // COMP_QUE: Disable comparator (default)

/**
 * @brief Writes a 16-bit value to an ADS1115 register.
 * @param fd File descriptor of the I2C device.
 * @param reg Register address.
 * @param value 16-bit value to write.
 * @return 0 if successful, -1 otherwise.
 */
static int ads1115_write_reg(int fd, uint8_t reg, uint16_t value) {
    uint8_t write_buf[3];
    write_buf[0] = reg;
    write_buf[1] = (uint8_t)(value >> 8);   // MSB
    write_buf[2] = (uint8_t)(value & 0xFF); // LSB

    if (write(fd, write_buf, 3) != 3) {
        fprintf(stderr, "Error writing to ADS1115 register 0x%02X: %s\n", reg, strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * @brief Reads a 16-bit value from an ADS1115 register.
 * @param fd File descriptor of the I2C device.
 * @param reg Register address.
 * @param value Pointer to store the read 16-bit value.
 * @return 0 if successful, -1 otherwise.
 */
static int ads1115_read_reg(int fd, uint8_t reg, uint16_t *value) {
    uint8_t read_buf[2];

    if (write(fd, &reg, 1) != 1) {
        fprintf(stderr, "Error writing register pointer to ADS1115: %s\n", strerror(errno));
        return -1;
    }

    if (read(fd, read_buf, 2) != 2) {
        fprintf(stderr, "Error reading from ADS1115 register 0x%02X: %s\n", reg, strerror(errno));
        return -1;
    }

    *value = (uint16_t)((read_buf[0] << 8) | read_buf[1]);
    return 0;
}

/**
 * @brief Initializes the ADS1115 ADC.
 * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-1").
 * @param address I2C address of the ADS1115 (e.g., ADS1115_ADDRESS_GND).
 * @return File descriptor if successful, -1 otherwise.
 */
int ads1115_init(const char *i2c_dev_path, uint8_t address) {
    int fd = open(i2c_dev_path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Error opening I2C device %s: %s\n", i2c_dev_path, strerror(errno));
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, address) < 0) {
        fprintf(stderr, "Error setting I2C slave address 0x%02X: %s\n", address, strerror(errno));
        close(fd);
        return -1;
    }

    // Default configuration: Single-shot, AIN0/GND, PGA +/-2.048V, 128SPS, Comparator disabled
    // This configuration will be modified by ads1115_read_adc_channel
    uint16_t config = ADS1115_CONFIG_OS_SINGLE | ADS1115_CONFIG_MUX_P0_GND | 
                      ADS1115_CONFIG_PGA_2_048V | ADS1115_CONFIG_MODE_SINGLE | 
                      ADS1115_CONFIG_DR_128SPS | ADS1115_CONFIG_COMP_QUE_DISABLE;
    
    if (ads1115_write_reg(fd, ADS1115_REG_POINTER_CONFIG, config) < 0) {
        close(fd);
        return -1;
    }

    printf("ADS1115 initialized on %s with address 0x%02X.\n", i2c_dev_path, address);
    return fd;
}

/**
 * @brief Reads an ADC channel from ADS1115.
 * @param fd File descriptor of the I2C device.
 * @param channel ADC channel to read (0-3 for single-ended, 0-3 for differential pairs).
 * @return ADC raw value (int16_t) if successful, -1 otherwise.
 */
int16_t ads1115_read_adc_channel(int fd, uint8_t channel) {
    uint16_t config = ADS1115_CONFIG_OS_SINGLE | ADS1115_CONFIG_PGA_2_048V | 
                      ADS1115_CONFIG_MODE_SINGLE | ADS1115_CONFIG_DR_128SPS | 
                      ADS1115_CONFIG_COMP_QUE_DISABLE;

    // Set MUX for selected channel (single-ended to GND)
    switch (channel) {
        case 0: config |= ADS1115_CONFIG_MUX_P0_GND; break;
        case 1: config |= ADS1115_CONFIG_MUX_P1_GND; break;
        case 2: config |= ADS1115_CONFIG_MUX_P2_GND; break;
        case 3: config |= ADS1115_CONFIG_MUX_P3_GND; break;
        default:
            fprintf(stderr, "Invalid ADS1115 channel: %d\n", channel);
            return -1;
    }

    // Write the configuration register
    if (ads1115_write_reg(fd, ADS1115_REG_POINTER_CONFIG, config) < 0) {
        return -1;
    }

    // Wait for the conversion to complete (max 1/128SPS = ~8ms)
    // You can also poll the OS bit in the config register
    usleep(10000); // Wait 10ms to be safe

    uint16_t raw_value;
    if (ads1115_read_reg(fd, ADS1115_REG_POINTER_CONVERT, &raw_value) < 0) {
        return -1;
    }

    // The raw value is signed 16-bit
    return (int16_t)raw_value;
}

/**
 * @brief Closes the ADS1115 I2C device.
 * @param fd File descriptor of the I2C device.
 */
void ads1115_close(int fd) {
    if (fd != -1) {
        close(fd);
        printf("ADS1115 I2C device closed.\n");
    }
}

// Example usage (for testing purposes)
// 示例用法（仅用于测试）
#ifdef ADS1115_TEST
int main() {
    int ads_fd;
    int16_t adc_value;
    const char *i2c_dev = "/dev/i2c-1"; // <--- 根据您的实际I2C设备路径进行调整
    uint8_t ads_address = ADS1115_ADDRESS_GND; // <--- 根据您的ADS1115的ADDR引脚连接调整此项

    ads_fd = ads1115_init(i2c_dev, ads_address);
    if (ads_fd < 0) {
        fprintf(stderr, "ADS1115 test failed to initialize.\n");
        return -1;
    }

    printf("Reading ADC channel 0 (connected to MQ-2)...\n"); // 正在读取ADC通道0（连接到MQ-2）。..
    for (int i = 0; i < 10; i++) {
        adc_value = ads1115_read_adc_channel(ads_fd, 0);
        if (adc_value != -1) {
            printf("ADC Channel 0 Raw Value: %d\n", adc_value);
        } else {
            fprintf(stderr, "Error reading ADC channel 0.\n");
        }
        usleep(500000); // Read every 500ms
    }

    ads1115_close(ads_fd);
    return 0;
}
#endif // ADS1115_TEST
