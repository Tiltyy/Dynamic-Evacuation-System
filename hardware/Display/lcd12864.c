/**
 * LCD12864显示模块实现
 * 适用于动态应急智能疏散系统
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include "lcd12864h.c"
#define LCD_WIDTH 128
#define LCD_HEIGHT 64

typedef enum {
    DIRECTION_UP = 0,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_UP_LEFT,
    DIRECTION_UP_RIGHT,
    DIRECTION_DOWN_LEFT,
    DIRECTION_DOWN_RIGHT
} direction_t;

typedef enum {
    LCD_ALERT_NONE = 0,
    LCD_ALERT_LOW,
    LCD_ALERT_MEDIUM,
    LCD_ALERT_HIGH
} lcd_alert_level_t;

// 显示缓冲区 (128x64 / 8 = 1024 bytes)
static uint8_t lcd_buffer[LCD_WIDTH * LCD_HEIGHT / 8];

// 箭头图案定义 (8x8像素)
static const uint8_t arrow_patterns[][8] = {
    // 上箭头
    {
        0x00, 0x00, 0x10, 0x38, 0x7C, 0x10, 0x10, 0x00
    },
    // 下箭头
    {
        0x00, 0x10, 0x10, 0x7C, 0x38, 0x10, 0x00, 0x00
    },
    // 左箭头
    {
        0x00, 0x10, 0x30, 0x7E, 0x7E, 0x30, 0x10, 0x00
    },
    // 右箭头
    {
        0x00, 0x08, 0x0C, 0x7E, 0x7E, 0x0C, 0x08, 0x00
    },
    // 左上箭头
    {
        0x00, 0x70, 0x38, 0x1C, 0x0E, 0x18, 0x30, 0x00
    },
    // 右上箭头
    {
        0x00, 0x0E, 0x1C, 0x38, 0x70, 0x18, 0x0C, 0x00
    },
    // 左下箭头
    {
        0x00, 0x30, 0x18, 0x0E, 0x1C, 0x38, 0x70, 0x00
    },
    // 右下箭头
    {
        0x00, 0x0C, 0x18, 0x70, 0x38, 0x1C, 0x0E, 0x00
    }
};

// 警报图标 (8x8像素)
static const uint8_t alert_icon[8] = {
    0x00, 0x18, 0x3C, 0x3C, 0x3C, 0x3C, 0x18, 0x00
};

// 电池图标 (16x8像素)
static const uint8_t battery_icon[2][8] = {
    {0x7F, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x7F},
    {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F}
};

// 字体定义 (5x7像素)
static const uint8_t font_5x7[][5] = {
    // 数字 0-9
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    
    // 字母 A-Z (简化版，仅包含常用字母)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x62, 0x51, 0x51, 0x51, 0x4E}, // S
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    // ... 其他字母可根据需要添加
    
    // 特殊字符
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 空格
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}  // /
};

// LCD控制函数（硬件相关，需根据实际硬件进行修改）
static int lcd_fd = -1;

// 初始化LCD12864
int lcd_init(void) {
    //初始化代码
    LCD_PSB=1;               //并口方式
   write_cmd(0x30);         //基本指令操作
   delay_1ms(5);
   write_cmd(0x0C);         //显示开，关光标
   delay_1ms(5);
   write_cmd(0x01);         //清除LCD的显示内容
   delay_1ms(5);
    
    printf("初始化LCD12864显示屏...\n");
    
    // 清空缓冲区
    memset(lcd_buffer, 0, sizeof(lcd_buffer));
    
    // 模拟打开设备
    // lcd_fd = open("/dev/lcd12864", O_RDWR);
    // if (lcd_fd < 0) {
    //     perror("无法打开LCD设备");
    //     return -1;
    // }
    
    // 设置初始对比度
    lcd_set_contrast(40);
    
    // 清屏
    lcd_clear();
    lcd_update();
    
    printf("LCD12864初始化成功\n");
    return 0;
}

// 清除LCD屏幕
void lcd_clear(void) {
    memset(lcd_buffer, 0, sizeof(lcd_buffer));
}

// 设置显示对比度
void lcd_set_contrast(uint8_t contrast) {
    // 限制对比度范围
    if (contrast > 63) {
        contrast = 63;
    }
    
   
    // 根据实际硬件的命令发送代码
    int fd;
    uint8_t cmd[2] = {0x28, contrast};
    
    // 打开I2C设备文件,LCD12864通过I2C接口连接
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C device");
        return;
    }
    
    // 设置I2C从机地址
    uint8_t lcd_addr = 0x27; 
    if (ioctl(fd, I2C_SLAVE, lcd_addr) < 0) {
        perror("Failed to set I2C address");
        close(fd);
        return;
    }
    
    // 发送对比度设置命令
    if (write(fd, cmd, 2) != 2) {
        perror("Failed to send contrast command");
    } else {
        printf("设置LCD对比度: %d (命令: 0x%02X 0x%02X)\n", contrast, cmd[0], cmd[1]);
    }
    
    close(fd);
}
    


// 在缓冲区设置像素点
static void lcd_set_pixel(uint8_t x, uint8_t y, uint8_t value) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }
    
    uint16_t byte_pos = (y / 8) * LCD_WIDTH + x;
    uint8_t bit_pos = y % 8;
    
    if (value) {
        lcd_buffer[byte_pos] |= (1 << bit_pos);
    } else {
        lcd_buffer[byte_pos] &= ~(1 << bit_pos);
    }
}

// 在缓冲区绘制字节图案
static void lcd_draw_byte(uint8_t x, uint8_t y, uint8_t pattern) {
    for (int i = 0; i < 8; i++) {
        lcd_set_pixel(x, y + i, (pattern >> i) & 0x01);
    }
}

// 在缓冲区绘制8x8图案
static void lcd_draw_pattern_8x8(uint8_t x, uint8_t y, const uint8_t pattern[8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            lcd_set_pixel(x + j, y + i, (pattern[i] >> (7 - j)) & 0x01);
        }
    }
}

// 在缓冲区绘制5x7字符
static void lcd_draw_char_5x7(uint8_t x, uint8_t y, char ch) {
    int char_index;
    
    // 映射字符到字体索引
    if (ch >= '0' && ch <= '9') {
        char_index = ch - '0';
    } else if (ch >= 'A' && ch <= 'Z') {
        char_index = 10 + (ch - 'A');
    } else if (ch >= 'a' && ch <= 'z') {
        char_index = 10 + (ch - 'a'); // 使用大写字母的字体
    } else if (ch == ' ') {
        char_index = 36;
    } else if (ch >= '!' && ch <= '/') {
        char_index = 37 + (ch - '!');
    } else {
        char_index = 36; // 默认为空格
    }
    
    // 绘制字符
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 5; j++) {
            if (char_index < sizeof(font_5x7) / sizeof(font_5x7[0])) {
                lcd_set_pixel(x + j, y + i, (font_5x7[char_index][j] >> i) & 0x01);
            }
        }
    }
}

// 在指定位置显示文本
void lcd_display_text(uint8_t x, uint8_t y, const char *text) {
    uint8_t pos_x = x;
    
    while (*text && pos_x < LCD_WIDTH - 5) {
        lcd_draw_char_5x7(pos_x, y, *text);
        text++;
        pos_x += 6; // 5像素宽度 + 1像素间距
    }
}

// 显示疏散方向指示箭头
void lcd_display_arrow(uint8_t x, uint8_t y, direction_t direction, uint8_t size) {
    if (direction > DIRECTION_DOWN_RIGHT || size < 1 || size > 3) {
        return;
    }
    
    // 绘制基本箭头
    lcd_draw_pattern_8x8(x, y, arrow_patterns[direction]);
    
    // 如果需要更大的箭头，绘制放大版本
    if (size > 1) {
        // 放大2倍或3倍的箭头实现
        // 这里简化处理，仅复制基本箭头到周围位置
        for (int i = 0; i < size - 1; i++) {
            lcd_draw_pattern_8x8(x + 8, y, arrow_patterns[direction]);
            lcd_draw_pattern_8x8(x, y + 8, arrow_patterns[direction]);
            lcd_draw_pattern_8x8(x + 8, y + 8, arrow_patterns[direction]);
        }
    }
}

// 显示警报状态
void lcd_display_alert(lcd_alert_level_t level, const char *message) {
    if (level == LCD_ALERT_NONE) {
        return;
    }
    
    // 绘制警报图标
    lcd_draw_pattern_8x8(0, 0, alert_icon);
    
    // 根据警报级别设置不同的显示效果
    switch (level) {
        case LCD_ALERT_HIGH:
            // 高级警报：显示"警告"和消息
            lcd_display_text(10, 0, "警告!");
            lcd_display_text(0, 16, message);
            break;
            
        case LCD_ALERT_MEDIUM:
            // 中级警报：显示"注意"和消息
            lcd_display_text(10, 0, "注意");
            lcd_display_text(0, 16, message);
            break;
            
        case LCD_ALERT_LOW:
            // 低级警报：仅显示消息
            lcd_display_text(10, 0, message);
            break;
            
        default:
            break;
    }
}

// 显示简单的区域地图
void lcd_display_map(int current_area_id, int exit_area_id) {
    // 简化的地图显示，仅显示当前位置和出口位置
    lcd_display_text(0, 0, "位置图");
    
    // 绘制简单的矩形框表示地图
    for (int i = 0; i < 40; i++) {
        lcd_set_pixel(i, 16, 1);
        lcd_set_pixel(i, 40, 1);
    }
    for (int i = 16; i <= 40; i++) {
        lcd_set_pixel(0, i, 1);
        lcd_set_pixel(39, i, 1);
    }
    
    // 显示当前位置
    char pos_text[16];
    snprintf(pos_text, sizeof(pos_text), "当前:%d", current_area_id);
    lcd_display_text(0, 48, pos_text);
    
    // 显示出口位置
    snprintf(pos_text, sizeof(pos_text), "出口:%d", exit_area_id);
    lcd_display_text(64, 48, pos_text);
    
    // 在地图上标记当前位置和出口
    // 这里使用简化的固定位置，实际应用中应根据区域ID计算实际坐标
    lcd_draw_pattern_8x8(10, 24, arrow_patterns[DIRECTION_UP]); // 当前位置
    lcd_draw_pattern_8x8(30, 24, arrow_patterns[DIRECTION_RIGHT]); // 出口位置
}

// 显示电池电量
void lcd_display_battery(uint8_t percentage) {
    // 限制百分比范围
    if (percentage > 100) {
        percentage = 100;
    }
    
    // 绘制电池外框
    lcd_draw_pattern_8x8(LCD_WIDTH - 16, 0, battery_icon[0]);
    
    // 绘制电池电量
    uint8_t fill_width = percentage * 14 / 100;
    for (int i = 0; i < fill_width && i < 14; i++) {
        for (int j = 0; j < 6; j++) {
            lcd_set_pixel(LCD_WIDTH - 15 + i, 1 + j, 1);
        }
    }
    
    // 显示电量百分比
    char batt_text[5];
    snprintf(batt_text, sizeof(batt_text), "%d%%", percentage);
    lcd_display_text(LCD_WIDTH - 30, 0, batt_text);
}

// 显示进度条
void lcd_display_progress_bar(uint8_t x, uint8_t y, uint8_t width, uint8_t percentage) {
    // 限制百分比范围
    if (percentage > 100) {
        percentage = 100;
    }
    
    // 绘制进度条外框
    for (int i = 0; i < width; i++) {
        lcd_set_pixel(x + i, y, 1);
        lcd_set_pixel(x + i, y + 6, 1);
    }
    lcd_set_pixel(x, y + 1, 1);
    lcd_set_pixel(x, y + 2, 1);
    lcd_set_pixel(x, y + 3, 1);
    lcd_set_pixel(x, y + 4, 1);
    lcd_set_pixel(x, y + 5, 1);
    lcd_set_pixel(x + width - 1, y + 1, 1);
    lcd_set_pixel(x + width - 1, y + 2, 1);
    lcd_set_pixel(x + width - 1, y + 3, 1);
    lcd_set_pixel(x + width - 1, y + 4, 1);
    lcd_set_pixel(x + width - 1, y + 5, 1);
    
    // 绘制进度条填充部分
    uint8_t fill_width = percentage * (width - 2) / 100;
    for (int i = 0; i < fill_width; i++) {
        for (int j = 0; j < 5; j++) {
            lcd_set_pixel(x + 1 + i, y + 1 + j, 1);
        }
    }
}

// 更新显示
void lcd_update(void) {
    // 将缓冲区内容发送到LCD的代码
    // I2C发送数据
    int f1c100s_i2c_send_data(int TWIx, u8 dat)
{
	write32(TWI_DATA_REG(TWIx), dat);
	C_BIT(TWI_CNTR_REG(TWIx),3);
	
	return f1c100s_i2c_wait_status(TWIx);
}
    // 模拟更新过程
    printf("更新LCD显示...\n");
    
    // 发送缓冲区数据到LCD控制器



ssize_t send_data_to_lcd(int lcd_fd, const void *lcd_buffer, size_t buffer_size) {
    // 检查参数有效性
    if (lcd_fd < 0 || lcd_buffer == NULL || buffer_size == 0) {
        errno = EINVAL;
        return -1;
    }

    // 检查设备是否可写
    if (fcntl(lcd_fd, F_GETFL) & O_WRONLY == 0) {
        errno = EBADF;
        return -1;
    }

    ssize_t total_bytes_written = 0;
    ssize_t bytes_written = 0;
    
    // 循环处理，确保所有数据都被写入（处理部分写入的情况）
    while (total_bytes_written < buffer_size) {
        bytes_written = write(lcd_fd, 
                             (const char*)lcd_buffer + total_bytes_written, 
                             buffer_size - total_bytes_written);
        
        if (bytes_written == -1) {
            // 处理中断情况（EINTR），可以重试
            if (errno == EINTR) {
                continue;
            }
            // 其他错误，记录日志并返回
            fprintf(stderr, "写入LCD失败: %s\n", strerror(errno));
            return -1;
        }
        
        total_bytes_written += bytes_written;
    }
    
    // 可选：等待LCD控制器处理完成（根据设备特性决定是否需要）
    // usleep(1000); // 延迟1ms
    
    return total_bytes_written;
}
     
}

// 关闭LCD
void lcd_close(void) {
    // 清屏
    lcd_clear();
    lcd_update();
    
    // 关闭设备
    if (lcd_fd >= 0) {
        // close(lcd_fd);
        lcd_fd = -1;
    }
    
    printf("LCD12864已关闭\n");
}
