#include "ui_module.h"
#include "../../hardware/drivers/ssd1306_linux/ssd1306.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../software/path_planning/path_planning.h"

// Global variable for buzzer GPIO pin
static int buzzer_gpio_pin = -1;

// 仅用于显示路径方向
int ui_init(int i2c_dev, int buzzer_pin) {
    buzzer_gpio_pin = buzzer_pin;
    // Placeholder for GPIO export and direction setting for buzzer
    // In a real system, you would export the GPIO pin and set its direction here.
    // For example: system("echo 2 > /sys/class/gpio/export");
    // system("echo out > /sys/class/gpio/gpio2/direction");

    if (ssd1306_init(i2c_dev) != 0) {
        fprintf(stderr, "Error: Failed to initialize SSD1306 OLED.\n");
        return -1;
    }
    if (ssd1306_oled_default_config(SSD1306_128_64_LINES, SSD1306_128_64_COLUMNS) != 0) {
        fprintf(stderr, "Error: Failed to configure SSD1306 OLED.\n");
        return -1;
    }
    ssd1306_oled_clear_screen();
    return 0;
}

// 根据路径数据显示方向箭头
int ui_update(const EnvironmentalData_t *env_data, const MotionData_t *motion_data, const path_t *path) {
    ssd1306_oled_clear_screen();

    if (path != NULL && path->num_nodes > 0) {
        // 假设 get_direction_from_path 返回一个整数代表方向
        int direction = get_direction_from_path(path);
        char arrow_char = ' ';
        switch (direction) {
            case 0: // East
                arrow_char = '>'; 
                break;
            case 1: // North
                arrow_char = '^'; 
                break;
            case 2: // West
                arrow_char = '<'; 
                break;
            case 3: // South
                arrow_char = 'v'; 
                break;
            default:
                arrow_char = '?'; // Unknown direction
                break;
        }
        char arrow_str[2];
        arrow_str[0] = arrow_char;
        arrow_str[1] = '\0'; // Null-terminate the string
        ssd1306_oled_write_line(SSD1306_FONT_NORMAL, arrow_str);
    } else {
        ssd1306_oled_write_line(SSD1306_FONT_NORMAL, "NO PATH");
    }

    return 0;
}

void ui_trigger_alert(int duration_ms) {
    if (buzzer_gpio_pin != -1) {
        // Placeholder for setting GPIO high to turn on buzzer
        // For example: system("echo 1 > /sys/class/gpio/gpio" + buzzer_gpio_pin + "/value");
        usleep(duration_ms * 1000);
        // Placeholder for setting GPIO low to turn off buzzer
        // For example: system("echo 0 > /sys/class/gpio/gpio" + buzzer_gpio_pin + "/value");
    }
}

void ui_cleanup(void) {
    ssd1306_oled_clear_screen();
    ssd1306_end();
    // Placeholder for GPIO unexport for buzzer
    // For example: system("echo 2 > /sys/class/gpio/unexport");
}


