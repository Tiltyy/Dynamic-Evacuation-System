#ifndef UI_MODULE_H
#define UI_MODULE_H

#include <stdint.h>
#include "../../software/path_planning/path_planning.h"
#include "../../hardware/drivers/ssd1306_linux/ssd1306.h"
#include "../../software/data_fusion/data_fusion.h"

// 函数原型
int ui_init(int i2c_dev, int buzzer_gpio_pin);
int ui_update(const EnvironmentalData_t *env_data, const MotionData_t *motion_data, const path_t *path);
void ui_cleanup(void);
void ui_trigger_alert(int duration_ms);

// 伪函数，需要根据实际情况实现
int get_direction_from_path(const path_t *path);

#endif // UI_MODULE_H


