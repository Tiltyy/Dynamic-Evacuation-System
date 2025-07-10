#ifndef ADS1115_DRIVER_H
#define ADS1115_DRIVER_H

#include <stdint.h>

// ADS1115 I2C addresses
// ADS1115 I2C地址
#define ADS1115_ADDRESS_GND 0x48 // ADDR pin connected to GND
#define ADS1115_ADDRESS_VCC 0x49 // ADDR pin connected to VCC
#define ADS1115_ADDRESS_SDA 0x4A // ADDR pin connected to SDA
#define ADS1115_ADDRESS_SCL 0x4B // ADDR pin connected to SCL

// ADS1115 Registers
// ADS1115寄存器
#define ADS1115_REG_POINTER_CONVERT 0x00 // Conversion register
#define ADS1115_REG_POINTER_CONFIG  0x01 // Configuration register
#define ADS1115_REG_POINTER_LOWTHRESH 0x02 // Low threshold register
#define ADS1115_REG_POINTER_HITHRESH 0x03 // High threshold register

// ADS1115 Configuration Register Bits
// ADS1115配置寄存器位
#define ADS1115_OS_SINGLE    0x8000  // OS: Start a single conversion
#define ADS1115_MUX_P0_N1    0x4000  // MUX: Input multiplexer P = AIN0, N = AIN1
#define ADS1115_MUX_P0_N3    0x5000  // MUX: Input multiplexer P = AIN0, N = AIN3
#define ADS1115_MUX_P1_N3    0x6000  // MUX: Input multiplexer P = AIN1, N = AIN3
#define ADS1115_MUX_P2_N3    0x7000  // MUX: Input multiplexer P = AIN2, N = AIN3
#define ADS1115_PGA_6_144V   0x0000  // PGA: +/-6.144V
#define ADS1115_PGA_4_096V   0x0200  // PGA: +/-4.096V
#define ADS1115_PGA_2_048V   0x0400  // PGA: +/-2.048V (default)
#define ADS1115_PGA_1_024V   0x0600  // PGA: +/-1.024V
#define ADS1115_PGA_0_512V   0x0800  // PGA: +/-0.512V
#define ADS1115_PGA_0_256V   0x0A00  // PGA: +/-0.256V
#define ADS1115_MODE_CONTIN  0x0000  // MODE: Continuous conversion mode
#define ADS1115_MODE_SINGLE  0x0100  // MODE: Single-shot mode
#define ADS1115_DR_8SPS      0x0000  // DR: Data rate 8 SPS
#define ADS1115_DR_16SPS     0x0020  // DR: Data rate 16 SPS
#define ADS1115_DR_32SPS     0x0040  // DR: Data rate 32 SPS
#define ADS1115_DR_64SPS     0x0060  // DR: Data rate 64 SPS
#define ADS1115_DR_128SPS    0x0080  // DR: Data rate 128 SPS (default)
#define ADS1115_DR_250SPS    0x00A0  // DR: Data rate 250 SPS
#define ADS1115_DR_475SPS    0x00C0  // DR: Data rate 475 SPS
#define ADS1115_DR_860SPS    0x00E0  // DR: Data rate 860 SPS
#define ADS1115_COMP_MODE_TRAD 0x0000 // COMP_MODE: Traditional comparator
#define ADS1115_COMP_MODE_WINDOW 0x0010 // COMP_MODE: Window comparator
#define ADS1115_COMP_POL_LOW   0x0000  // COMP_POL: Active low
#define ADS1115_COMP_POL_HIGH  0x0008  // COMP_POL: Active high
#define ADS1115_COMP_LAT_NONLAT 0x0000 // COMP_LAT: Non-latching comparator
#define ADS1115_COMP_LAT_LATCH 0x0004  // COMP_LAT: Latching comparator
#define ADS1115_COMP_QUE_1CONV 0x0000  // COMP_QUE: Assert after 1 conversion
#define ADS1115_COMP_QUE_2CONV 0x0001  // COMP_QUE: Assert after 2 conversions
#define ADS1115_COMP_QUE_4CONV 0x0002  // COMP_QUE: Assert after 4 conversions
#define ADS1115_COMP_QUE_DISABLE 0x0003 // COMP_QUE: Disable comparator

// Function prototypes
// 函数原型
/**
 * @brief Initializes the ADS1115 ADC.
 * @param i2c_dev_path Path to the I2C device (e.g., "/dev/i2c-1").
 * @param address I2C address of the ADS1115 (e.g., ADS1115_ADDRESS_GND).
 * @return File descriptor if successful, -1 otherwise.
 */
int ads1115_init(const char *i2c_dev_path, uint8_t address);

/**
 * @brief Reads a single ADC channel from ADS1115.
 * @param fd File descriptor of the I2C device.
 * @param channel The ADC channel to read (0-3).
 * @return The 16-bit ADC raw value, or -1 on error.
 */
int16_t ads1115_read_adc_channel(int fd, uint8_t channel);

/**
 * @brief Closes the ADS1115 I2C device.
 * @param fd File descriptor of the I2C device.
 */
void ads1115_close(int fd);

#endif // ADS1115_DRIVER_H


