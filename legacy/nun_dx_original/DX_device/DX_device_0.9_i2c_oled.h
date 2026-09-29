/**
 * @file    DX_device_0.9_i2c_oled.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   0.9 寸 I2C OLED 显示屏驱动头文件
 *          提供 SSD1306 控制的 128x64 OLED 显示接口
 */

#ifndef __DX_DEVICE_0_9_I2C_OLED_H
#define __DX_DEVICE_0_9_I2C_OLED_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_OLED

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"

/* Exported defines ----------------------------------------------------------*/

#define OLED_ADDR       0x3C               /* OLED I2C 设备地址(7 位)     */
#define OLED_WIDTH      128                /* OLED 屏幕宽度(像素)         */
#define OLED_HEIGHT     64                 /* OLED 屏幕高度(像素)         */
#define OLED_PAGES      (OLED_HEIGHT / 8)  /* OLED 分页数(每页 8 行像素)  */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief OLED 字体尺寸枚举
 *        用于选择显示字符时使用的字模大小
 */
typedef enum
{
    OLED_FONT_6X8,      /* 6x8 小号字体  */
    OLED_FONT_8X16,     /* 8x16 大号字体 */
} oled_font_enum;

/* Exported functions prototypes ---------------------------------------------*/

void oled_init            (void);  /* OLED 初始化                              */
void oled_clear           (void);  /* 清空显存缓冲区                           */
void oled_refresh         (void);  /* 刷新显存到 OLED 屏幕                     */
void oled_show_char       (uint8 x, uint8 y, char ch, oled_font_enum font);  /* 显示单个字符    */
void oled_show_string     (uint8 x, uint8 y, const char *str, oled_font_enum font);  /* 显示字符串      */
void oled_show_num        (uint8 x, uint8 y, uint32 num, uint8 len, oled_font_enum font);  /* 显示无符号整数  */
void oled_show_float      (uint8 x, uint8 y, float num, uint8 int_len, uint8 dec_len, oled_font_enum font);  /* 显示浮点数      */
void oled_draw_point      (uint8 x, uint8 y, uint8 mode);  /* 绘制单个像素点                            */
void oled_draw_point      (uint8 x, uint8 y, uint8 mode);
void oled_test            (void);  /* OLED 测试函数                            */
#ifdef __cplusplus
}
#endif

#endif /* DX_USE_OLED */
#endif /* __DX_DEVICE_0_9_I2C_OLED_H */
