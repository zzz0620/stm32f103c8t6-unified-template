/**
 * @file    DX_device_st7789.h
 * @author  YCZ
 * @date    2026-08-04
 * @brief   ST7789 彩色液晶屏驱动头文件
 *          基于 SPI 总线封装 240x240 RGB565 彩色屏的初始化与绘图接口
 */

#ifndef __DX_DEVICE_ST7789_H
#define __DX_DEVICE_ST7789_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_ST7789

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_spi.h"
#include "DX_driver_gpio.h"
#include "DX_common_font.h"

/* Exported defines ----------------------------------------------------------*/

#define ST7789_WIDTH    240     /* 屏幕宽度(像素) */
#define ST7789_HEIGHT   240     /* 屏幕高度(像素) */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ST7789 字体尺寸枚举
 *        复用工程字库(DX_common_font)中的 6x8 与 8x16 字体
 */
typedef enum
{
    ST7789_FONT_6X8,        /* 6x8 小号字体   */
    ST7789_FONT_8X16,       /* 8x16 大号字体  */
} st7789_font_enum;

/**
 * @brief ST7789 硬件配置结构体
 *        描述 SPI 参数及 CS/DC/RST/BL 控制引脚
 */
typedef struct
{
    spi_index_enum spi;         /* SPI 外设索引         */
    spi_mode_enum  mode;        /* SPI 工作模式         */
    uint32         prescaler;   /* SPI 波特率分频系数    */
    spi_pin_enum   pin;         /* SPI 引脚配置         */

    GPIO_TypeDef  *cs_port;     /* CS 片选端口          */
    uint16         cs_pin;      /* CS 片选引脚          */
    GPIO_TypeDef  *dc_port;     /* DC 数据/命令端口     */
    uint16         dc_pin;      /* DC 数据/命令引脚     */
    GPIO_TypeDef  *rst_port;    /* RST 复位端口         */
    uint16         rst_pin;     /* RST 复位引脚         */
    GPIO_TypeDef  *bl_port;     /* BL 背光端口          */
    uint16         bl_pin;      /* BL 背光引脚          */
} st7789_cfg_t;

/* Exported functions prototypes ---------------------------------------------*/

void st7789_init        (const st7789_cfg_t *cfg);                                       /* 初始化屏幕                 */
void st7789_set_window  (uint16 x1, uint16 y1, uint16 x2, uint16 y2);                     /* 设置刷屏窗口               */
void st7789_clear       (uint16 color);                                                  /* 清屏(填充指定颜色)         */
void st7789_draw_pixel  (uint16 x, uint16 y, uint16 color);                               /* 绘制单个像素               */
void st7789_fill_rect   (uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 color);       /* 填充矩形区域               */
void st7789_draw_bitmap (uint16 x, uint16 y, uint16 width, uint16 height, const uint8 *data);  /* 绘制位图(RGB565)   */
void st7789_show_char   (uint16 x, uint16 y, char ch, st7789_font_enum font, uint16 color, uint16 bgcolor);  /* 显示单个字符 */
void st7789_show_string (uint16 x, uint16 y, const char *str, st7789_font_enum font, uint16 color, uint16 bgcolor);  /* 显示字符串 */
void st7789_test        (void);                                                          /* 测试函数                   */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_ST7789 */
#endif /* __DX_DEVICE_ST7789_H */
