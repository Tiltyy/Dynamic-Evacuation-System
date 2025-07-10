#ifndef _LINUX_I2C_H
#define _LINUX_I2C_H

#include <stdint.h>

extern uint8_t _i2c_init(int i2c, int dev_addr);
extern uint8_t _i2c_close();
extern uint8_t _i2c_write(uint8_t* ptr, int16_t len);
extern uint8_t _i2c_read(uint8_t *ptr, int16_t len);

#endif
