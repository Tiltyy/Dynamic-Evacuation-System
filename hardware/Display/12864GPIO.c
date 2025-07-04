#ifndef __12864_H__
#define __12864_H__

#include <stdint.h>
#include <stdbool.h>

// 龙芯2K1000平台GPIO控制接口
typedef enum {
    GPIO_LOW = 0,
    GPIO_HIGH = 1
} GpioLevel;

typedef enum {
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1
} GpioDirection;

// GPIO操作函数声明
int  gpio_init(void);
void gpio_set_direction(uint8_t pin, GpioDirection dir);
void gpio_set_level(uint8_t pin, GpioLevel level);
GpioLevel gpio_get_level(uint8_t pin);
void gpio_cleanup(void);

// 延时函数
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

// LCD12864接口定义 - 根据实际硬件连接修改
#define LCD_RS_PIN      10  // P3.2 -> GPIO10
#define LCD_RW_PIN      11  // P3.1 -> GPIO11
#define LCD_EN_PIN      12  // P3.0 -> GPIO12
#define LCD_CS1_PIN     13  // P3.4 -> GPIO13
#define LCD_CS2_PIN     14  // P3.3 -> GPIO14

// 数据线引脚定义 (D0-D7)
#define LCD_D0_PIN      0   // P2.0 -> GPIO0
#define LCD_D1_PIN      1   // P2.1 -> GPIO1
#define LCD_D2_PIN      2   // P2.2 -> GPIO2
#define LCD_D3_PIN      3   // P2.3 -> GPIO3
#define LCD_D4_PIN      4   // P2.4 -> GPIO4
#define LCD_D5_PIN      5   // P2.5 -> GPIO5
#define LCD_D6_PIN      6   // P2.6 -> GPIO6
#define LCD_D7_PIN      7   // P2.7 -> GPIO7

// 屏幕选择参数
#define SCREEN_FULL     0   // 全屏
#define SCREEN_LEFT     1   // 左屏
#define SCREEN_RIGHT    2   // 右屏

// 字模数组
extern const uint8_t Hzk[];
extern const uint8_t Szk[];
extern const uint8_t Fhk[];
extern const uint8_t Zmk[];

// 函数声明
void LCD_CheckState(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SetLine(uint8_t page);
void LCD_SetStartLine(uint8_t startline);
void LCD_SetColumn(uint8_t column);
void LCD_SetOnOff(bool on);
void LCD_WriteByte(uint8_t data);
void LCD_SelectScreen(uint8_t screen);
void LCD_ClearScreen(uint8_t screen);
int  LCD_Init(void);
void LCD_DisplayHZ(uint8_t screen, uint8_t page, uint8_t column, uint8_t number);
void LCD_DisplaySZ(uint8_t screen, uint8_t page, uint8_t column, uint8_t number);
void LCD_DisplayZM(uint8_t screen, uint8_t page, uint8_t column, uint8_t number);
void LCD_DisplayFH(uint8_t screen, uint8_t page, uint8_t column, uint8_t number);

// 扩展功能 - 显示字符串
void LCD_DisplayString(uint8_t screen, uint8_t page, uint8_t column, const char *str);

#endif /* __12864_H__ */
#include "lcd12864h.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// 龙芯2K1000 GPIO控制器基地址
#define GPIO_BASE_ADDRESS     0x13000000
#define GPIO_REG_SIZE         0x1000

// GPIO寄存器偏移量 
#define GPIO_DIRECTION_OFFSET 0x0004  // 方向寄存器
#define GPIO_OUTPUT_OFFSET    0x0008  // 输出寄存器
#define GPIO_INPUT_OFFSET     0x0000  // 输入寄存器

static volatile uint32_t *gpio_base = NULL;

// GPIO初始化
int gpio_init(void) {
    int fd;
    
    // 打开内存映射设备
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("无法打开/dev/mem");
        return -1;
    }
    
    // 映射GPIO控制器内存
    gpio_base = (volatile uint32_t *)mmap(
        NULL, GPIO_REG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE_ADDRESS);
    
    if (gpio_base == MAP_FAILED) {
        perror("内存映射失败");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

// 设置GPIO方向
void gpio_set_direction(uint8_t pin, GpioDirection dir) {
    uint32_t bank = pin / 32;
    uint32_t bit = pin % 32;
    uint32_t reg_offset = GPIO_DIRECTION_OFFSET + (bank * 0x100);
    
    if (dir == GPIO_OUTPUT) {
        gpio_base[reg_offset / 4] |= (1 << bit);
    } else {
        gpio_base[reg_offset / 4] &= ~(1 << bit);
    }
}

// 设置GPIO电平
void gpio_set_level(uint8_t pin, GpioLevel level) {
    uint32_t bank = pin / 32;
    uint32_t bit = pin % 32;
    uint32_t reg_offset = GPIO_OUTPUT_OFFSET + (bank * 0x100);
    
    if (level == GPIO_HIGH) {
        gpio_base[reg_offset / 4] |= (1 << bit);
    } else {
        gpio_base[reg_offset / 4] &= ~(1 << bit);
    }
}

// 获取GPIO电平
GpioLevel gpio_get_level(uint8_t pin) {
    uint32_t bank = pin / 32;
    uint32_t bit = pin % 32;
    uint32_t reg_offset = GPIO_INPUT_OFFSET + (bank * 0x100);
    
    return (gpio_base[reg_offset / 4] & (1 << bit)) ? GPIO_HIGH : GPIO_LOW;
}

// 清理GPIO资源
void gpio_cleanup(void) {
    if (gpio_base != NULL) {
        munmap((void *)gpio_base, GPIO_REG_SIZE);
        gpio_base = NULL;
    }
}

// 延时函数实现
void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void delay_us(uint32_t us) {
    usleep(us);
}

// 设置8位数据总线的值
static void set_data_bus(uint8_t data) {
    gpio_set_level(LCD_D0_PIN, (data & 0x01) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D1_PIN, (data & 0x02) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D2_PIN, (data & 0x04) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D3_PIN, (data & 0x08) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D4_PIN, (data & 0x10) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D5_PIN, (data & 0x20) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D6_PIN, (data & 0x40) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_level(LCD_D7_PIN, (data & 0x80) ? GPIO_HIGH : GPIO_LOW);
}

// 检查LCD状态 (判忙)
void LCD_CheckState(void) {
    uint8_t status;
    
    // 设置数据线为输入模式
    gpio_set_direction(LCD_D0_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D1_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D2_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D3_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D4_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D5_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D6_PIN, GPIO_INPUT);
    gpio_set_direction(LCD_D7_PIN, GPIO_INPUT);
    
    gpio_set_level(LCD_RS_PIN, GPIO_LOW);  // RS=0: 指令模式
    gpio_set_level(LCD_RW_PIN, GPIO_HIGH); // RW=1: 读模式
    
    do {
        gpio_set_level(LCD_EN_PIN, GPIO_HIGH);
        delay_us(5);
        
        // 读取DB7 (忙标志)
        status = gpio_get_level(LCD_D7_PIN);
        
        gpio_set_level(LCD_EN_PIN, GPIO_LOW);
        delay_us(5);
    } while (status);
    
    // 恢复数据线为输出模式
    gpio_set_direction(LCD_D0_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D1_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D2_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D3_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D4_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D5_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D6_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D7_PIN, GPIO_OUTPUT);
}

// 发送命令到LCD
void LCD_SendCommand(uint8_t cmd) {
    LCD_CheckState();
    
    gpio_set_level(LCD_RS_PIN, GPIO_LOW);  // RS=0: 指令模式
    gpio_set_level(LCD_RW_PIN, GPIO_LOW);  // RW=0: 写模式
    
    set_data_bus(cmd);
    
    // 产生EN下降沿
    gpio_set_level(LCD_EN_PIN, GPIO_HIGH);
    delay_us(5);
    gpio_set_level(LCD_EN_PIN, GPIO_LOW);
    delay_us(5);
}

// 发送数据到LCD
void LCD_WriteByte(uint8_t data) {
    LCD_CheckState();
    
    gpio_set_level(LCD_RS_PIN, GPIO_HIGH); // RS=1: 数据模式
    gpio_set_level(LCD_RW_PIN, GPIO_LOW);  // RW=0: 写模式
    
    set_data_bus(data);
    
    // 产生EN下降沿
    gpio_set_level(LCD_EN_PIN, GPIO_HIGH);
    delay_us(5);
    gpio_set_level(LCD_EN_PIN, GPIO_LOW);
    delay_us(5);
}

// 设置显示页 (0-7)
void LCD_SetLine(uint8_t page) {
    page = 0xB8 | (page & 0x07);  // 页地址命令
    LCD_SendCommand(page);
}

// 设置起始行 (0-63)
void LCD_SetStartLine(uint8_t startline) {
    startline = 0xC0 | (startline & 0x3F);  // 起始行命令
    LCD_SendCommand(startline);
}

// 设置列地址 (0-63)
void LCD_SetColumn(uint8_t column) {
    column = 0x40 | (column & 0x3F);  // 列地址命令
    LCD_SendCommand(column);
}

// 开关显示
void LCD_SetOnOff(bool on) {
    LCD_SendCommand(on ? 0x3F : 0x3E);
}

// 选择屏幕
void LCD_SelectScreen(uint8_t screen) {
    switch (screen) {
        case SCREEN_FULL:
            gpio_set_level(LCD_CS1_PIN, GPIO_LOW);
            gpio_set_level(LCD_CS2_PIN, GPIO_LOW);
            break;
        case SCREEN_LEFT:
            gpio_set_level(LCD_CS1_PIN, GPIO_HIGH);
            gpio_set_level(LCD_CS2_PIN, GPIO_LOW);
            break;
        case SCREEN_RIGHT:
            gpio_set_level(LCD_CS1_PIN, GPIO_LOW);
            gpio_set_level(LCD_CS2_PIN, GPIO_HIGH);
            break;
        default:
            break;
    }
}

// 清屏
void LCD_ClearScreen(uint8_t screen) {
    uint8_t i, j;
    
    LCD_SelectScreen(screen);
    
    for (i = 0; i < 8; i++) {
        LCD_SetLine(i);
        LCD_SetColumn(0);
        
        for (j = 0; j < 64; j++) {
            LCD_WriteByte(0x00);
        }
    }
}

// LCD初始化
int LCD_Init(void) {
    // 初始化GPIO
    if (gpio_init() < 0) {
        return -1;
    }
    
    // 设置LCD控制引脚为输出模式
    gpio_set_direction(LCD_RS_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_RW_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_EN_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_CS1_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_CS2_PIN, GPIO_OUTPUT);
    
    // 设置LCD数据引脚为输出模式
    gpio_set_direction(LCD_D0_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D1_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D2_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D3_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D4_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D5_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D6_PIN, GPIO_OUTPUT);
    gpio_set_direction(LCD_D7_PIN, GPIO_OUTPUT);
    
    // 延时等待LCD上电稳定
    delay_ms(50);
    
    // 初始化LCD
    LCD_SelectScreen(SCREEN_FULL);
    LCD_SetOnOff(false);  // 关显示
    delay_ms(5);
    
    LCD_SetOnOff(true);   // 开显示
    delay_ms(5);
    
    LCD_ClearScreen(SCREEN_FULL);
    LCD_SetStartLine(0);
    
    return 0;
}

// 显示汉字
void LCD_DisplayHZ(uint8_t screen, uint8_t page, uint8_t column, uint8_t number) {
    int i;
    
    LCD_SelectScreen(screen);
    column &= 0x3F;
    
    // 显示上半部分
    LCD_SetLine(page);
    LCD_SetColumn(column);
    
    for (i = 0; i < 16; i++) {
        LCD_WriteByte(Hzk[i + 32 * number]);
    }
    
    // 显示下半部分
    LCD_SetLine(page + 1);
    LCD_SetColumn(column);
    
    for (i = 0; i < 16; i++) {
        LCD_WriteByte(Hzk[i + 32 * number + 16]);
    }
}

// 显示数字
void LCD_DisplaySZ(uint8_t screen, uint8_t page, uint8_t column, uint8_t number) {
    int i;
    
    LCD_SelectScreen(screen);
    column &= 0x3F;
    
    // 显示上半部分
    LCD_SetLine(page);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Szk[i + 16 * number]);
    }
    
    // 显示下半部分
    LCD_SetLine(page + 1);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Szk[i + 16 * number + 8]);
    }
}

// 显示字母
void LCD_DisplayZM(uint8_t screen, uint8_t page, uint8_t column, uint8_t number) {
    int i;
    
    LCD_SelectScreen(screen);
    column &= 0x3F;
    
    // 显示上半部分
    LCD_SetLine(page);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Zmk[i + 16 * number]);
    }
    
    // 显示下半部分
    LCD_SetLine(page + 1);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Zmk[i + 16 * number + 8]);
    }
}

// 显示符号
void LCD_DisplayFH(uint8_t screen, uint8_t page, uint8_t column, uint8_t number) {
    int i;
    
    LCD_SelectScreen(screen);
    column &= 0x3F;
    
    // 显示上半部分
    LCD_SetLine(page);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Fhk[i + 16 * number]);
    }
    
    // 显示下半部分
    LCD_SetLine(page + 1);
    LCD_SetColumn(column);
    
    for (i = 0; i < 8; i++) {
        LCD_WriteByte(Fhk[i + 16 * number + 8]);
    }
}

// 显示字符串 (扩展功能)
void LCD_DisplayString(uint8_t screen, uint8_t page, uint8_t column, const char *str) {
    uint8_t i = 0;
    
    while (str[i] != '\0') {
        if (str[i] >= '0' && str[i] <= '9') {
            // 显示数字
            LCD_DisplaySZ(screen, page, column, str[i] - '0');
            column += 8;  // 数字宽度为8列
        } else if ((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z')) {
            // 显示字母
            uint8_t index = (str[i] >= 'a') ? (str[i] - 'a' + 26) : (str[i] - 'A');
            LCD_DisplayZM(screen, page, column, index);
            column += 8;  // 字母宽度为8列
        } else {
            // 显示符号或其他字符
            // 这里简化处理，实际应用中可能需要更完善的字符映射
            LCD_DisplayFH(screen, page, column, 0);  // 默认显示空格
            column += 8;
        }
        
        i++;
    }
}

#include "12864.h"

// 汉字字模数据
const uint8_t Hzk[] = {
    /*--  文字:  单  --*/
    0x00,0x00,0xF8,0x49,0x4A,0x4C,0x48,0xF8,0x48,0x4C,0x4A,0x49,0xF8,0x00,0x00,0x00,
    0x10,0x10,0x13,0x12,0x12,0x12,0x12,0xFF,0x12,0x12,0x12,0x12,0x13,0x10,0x10,0x00,
    
    /*--  文字:  片  --*/
    0x00,0x00,0x00,0xFE,0x20,0x20,0x20,0x20,0x20,0x3F,0x20,0x20,0x20,0x20,0x00,0x00,
    0x00,0x80,0x60,0x1F,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x00,
    
    /*--  文字:  机  --*/
    0x10,0x10,0xD0,0xFF,0x90,0x10,0x00,0xFE,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,
    0x04,0x03,0x00,0xFF,0x00,0x83,0x60,0x1F,0x00,0x00,0x00,0x3F,0x40,0x40,0x78,0x00,
    
    // 其他汉字字模...
};

// 数字字模数据
const uint8_t Szk[] = {
    /*--  文字:  0  --*/
    0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00,
    
    // 其他数字字模...
};

// 符号字模数据
const uint8_t Fhk[] = {
    /*--  文字:  +  --*/
    0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x0F,0x01,0x01,0x01,
    
    // 其他符号字模...
};

// 字母字模数据
const uint8_t Zmk[] = {
    /*--  文字:  A  --*/
    0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20,
    
    // 其他字母字模...
};