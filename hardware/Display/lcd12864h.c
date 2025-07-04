/**
 * LCD12864显示模块头文件
 * 适用于动态应急智能疏散系统
 */

#ifndef LCD12864_H
#define LCD12864_H

#include <stdint.h>

// LCD尺寸定义
#define LCD_WIDTH  128
#define LCD_HEIGHT 64

// 方向定义
typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_UP_LEFT,
    DIRECTION_UP_RIGHT,
    DIRECTION_DOWN_LEFT,
    DIRECTION_DOWN_RIGHT
} direction_t;

// 警报级别
typedef enum {
    LCD_ALERT_NONE,
    LCD_ALERT_LOW,
    LCD_ALERT_MEDIUM,
    LCD_ALERT_HIGH
} lcd_alert_level_t;

/**
 * 初始化LCD12864
 * 
 * @return 成功返回0，失败返回-1
 */
int lcd_init(void);

/**
 * 清除LCD屏幕
 */
void lcd_clear(void);

/**
 * 设置显示对比度
 * 
 * @param contrast 对比度值(0-63)
 */
void lcd_set_contrast(uint8_t contrast);

/**
 * 在指定位置显示文本
 * 
 * @param x X坐标(0-127)
 * @param y Y坐标(0-63)，通常按8的倍数
 * @param text 要显示的文本
 */
void lcd_display_text(uint8_t x, uint8_t y, const char *text);

/**
 * 显示疏散方向指示箭头
 * 
 * @param x X坐标(0-127)
 * @param y Y坐标(0-63)
 * @param direction 箭头方向
 * @param size 箭头大小(1-3)
 */
void lcd_display_arrow(uint8_t x, uint8_t y, direction_t direction, uint8_t size);

/**
 * 显示警报状态
 * 
 * @param level 警报级别
 * @param message 简短警报消息
 */
void lcd_display_alert(lcd_alert_level_t level, const char *message);

/**
 * 显示简单的区域地图
 * 
 * @param current_area_id 当前区域ID
 * @param exit_area_id 出口区域ID
 */
void lcd_display_map(int current_area_id, int exit_area_id);

/**
 * 显示电池电量
 * 
 * @param percentage 电量百分比(0-100)
 */
void lcd_display_battery(uint8_t percentage);

/**
 * 显示进度条
 * 
 * @param x X坐标(0-127)
 * @param y Y坐标(0-63)
 * @param width 进度条宽度
 * @param percentage 进度百分比(0-100)
 */
void lcd_display_progress_bar(uint8_t x, uint8_t y, uint8_t width, uint8_t percentage);

/**
 * 更新显示
 * 将缓冲区内容刷新到LCD
 */
void lcd_update(void);

/**
 * 关闭LCD
 */
void lcd_close(void);

#endif /* LCD12864_H */
