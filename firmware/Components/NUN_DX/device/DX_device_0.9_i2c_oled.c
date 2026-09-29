#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_OLED

/**
 * @file    DX_device_0.9_i2c_oled.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   0.9 寸 I2C OLED 显示屏驱动源文件
 *          基于 I2C 驱动实现 SSD1306 OLED 的初始化、显存刷新与字符/数值显示
 */

#include "DX_device_0.9_i2c_oled.h"
#include "DX_driver_i2c.h"
#include "DX_driver_delay.h"
#include "DX_common_font.h"

#define OLED_CTRL_CMD   0x00    /* 控制字节:后续为命令         */
#define OLED_CTRL_DATA  0x40    /* 控制字节:后续为数据         */

static uint8 oled_buffer[OLED_PAGES][OLED_WIDTH];  /* OLED 显存缓冲区,按页与列组织 */

/* OLED 设备句柄:总线、地址、超时一处定义,总线由应用层 i2c_init 初始化 */
static const i2c_dev_t oled_dev = { TEMPLATE_DX_I2C_BUS, OLED_ADDR, 100 };

/**
 * @brief  向 OLED 写入单条命令
 * @param  cmd: 命令字节
 * @retval 无
 */
static void oled_write_cmd (uint8 cmd)
{
    i2c_dev_mem_write(&oled_dev, OLED_CTRL_CMD, &cmd, 1);  /* 通过 I2C 写入命令 */
}

/**
 * @brief  向 OLED 写入数据
 * @param  dat: 数据指针
 * @param  len: 数据长度
 * @retval 无
 */
static void oled_write_data (const uint8 *dat, uint16 len)
{
    i2c_dev_mem_write(&oled_dev, OLED_CTRL_DATA, (uint8 *)dat, len);  /* 通过 I2C 写入数据 */
}

/**
 * @brief  获取字体的宽高信息
 * @param  font: 字体枚举
 * @param  w   : 输出字体宽度(像素)
 * @param  h   : 输出字体高度(像素)
 * @retval 无
 */
static void oled_font_info (oled_font_enum font, uint8 *w, uint8 *h)
{
    *w = (font == OLED_FONT_6X8) ? 6 : 8;   /* 6x8 宽 6,8x16 宽 8  */
    *h = (font == OLED_FONT_6X8) ? 8 : 16;  /* 6x8 高 8,8x16 高 16 */
}

/**
 * @brief  OLED 初始化
 *         配置 I2C 并发送 SSD1306 初始化命令序列,完成后清屏并刷新
 * @retval 无
 */
void oled_init (void)
{
    uint8 init_cmds[] =  /* SSD1306 初始化命令序列 */
    {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };

    for (uint8 i = 0; i < sizeof(init_cmds); i++)
        oled_write_cmd(init_cmds[i]);  /* 依次写入初始化命令 */

    oled_clear();     /* 清空显存 */
    oled_refresh();   /* 刷新到屏幕 */
}

/**
 * @brief  清空显存缓冲区
 *         将整个显存缓冲区置 0
 * @retval 无
 */
void oled_clear (void)
{
    memset(oled_buffer, 0, sizeof(oled_buffer));  /* 显存清零 */
}

/**
 * @brief  刷新显存到 OLED 屏幕
 *         逐页设置显示地址并写入整页数据
 * @retval 无
 */
void oled_refresh (void)
{
    for (uint8 page = 0; page < OLED_PAGES; page++)
    {
        oled_write_cmd(0xB0 + page);  /* 设置页地址   */
        oled_write_cmd(0x00);         /* 设置列低地址 */
        oled_write_cmd(0x10);         /* 设置列高地址 */
        oled_write_data(oled_buffer[page], OLED_WIDTH);  /* 写入整页数据 */
    }
}

/**
 * @brief  在指定位置显示单个字符
 * @param  x   : 列坐标(像素)
 * @param  y   : 行坐标(像素,需为 8 的整数倍对齐)
 * @param  ch  : 待显示字符(ASCII)
 * @param  font: 字体尺寸
 * @retval 无
 * @note   字模取自 DX_common_font.h 中的 ascii_font_6x8 / ascii_font_8x16
 */
void oled_show_char (uint8 x, uint8 y, char ch, oled_font_enum font)
{
    uint8 cw, ch_h;
    oled_font_info(font, &cw, &ch_h);  /* 获取字体宽高 */
    if (x > OLED_WIDTH - cw || y + ch_h > OLED_HEIGHT) return;  /* 越界判断 */

    uint8 page = y / 8;           /* 计算所在页 */
    uint8 idx  = (uint8)ch - 32;  /* ASCII 偏移(跳过前 32 个控制字符) */

    if (font == OLED_FONT_6X8)
    {
        for (uint8 i = 0; i < 6; i++)
            oled_buffer[page][x + i] = ascii_font_6x8[idx][i];  /* 写入 6x8 字模 */
    }
    else
    {
        for (uint8 i = 0; i < 8; i++)
        {
            oled_buffer[page][x + i]     = ascii_font_8x16[idx][i];      /* 上半部分字模 */
            oled_buffer[page + 1][x + i] = ascii_font_8x16[idx][i + 8];  /* 下半部分字模 */
        }
    }
}

/**
 * @brief  在指定位置显示字符串
 * @param  x   : 起始列坐标
 * @param  y   : 起始行坐标
 * @param  str : 待显示字符串
 * @param  font: 字体尺寸
 * @retval 无
 * @note   超出屏宽自动换行,超出屏高停止显示
 */
void oled_show_string (uint8 x, uint8 y, const char *str, oled_font_enum font)
{
    uint8 cw, ch_h;
    oled_font_info(font, &cw, &ch_h);  /* 获取字体宽高 */

    while (*str)
    {
        oled_show_char(x, y, *str++, font);  /* 逐字符显示 */
        x += cw;                             /* 列坐标右移一个字宽 */
        if (x > OLED_WIDTH - cw) { x = 0; y += ch_h; }  /* 超宽换行 */
        if (y + ch_h > OLED_HEIGHT) break;              /* 超高退出 */
    }
}

/**
 * @brief  显示无符号整数
 * @param  x   : 起始列坐标
 * @param  y   : 起始行坐标
 * @param  num : 待显示数值
 * @param  len : 显示位数(不足前补空格,超出截断高位)
 * @param  font: 字体尺寸
 * @retval 无
 */
void oled_show_num (uint8 x, uint8 y, uint32 num, uint8 len, oled_font_enum font)
{
    char buf[12];
    uint8 i;
    for (i = 0; i < len; i++) buf[i] = ' ';  /* 预填充空格 */
    buf[len] = '\0';                          /* 字符串结束符 */

    i = len;
    while (i--)
    {
        buf[i] = '0' + (num % 10);  /* 低位转字符 */
        num /= 10;                  /* 右移一位   */
    }
    oled_show_string(x, y, buf, font);  /* 显示数值字符串 */
}

/**
 * @brief  显示浮点数
 * @param  x      : 起始列坐标
 * @param  y      : 起始行坐标
 * @param  num    : 待显示浮点数
 * @param  int_len: 整数部分位数
 * @param  dec_len: 小数部分位数(为 0 时不显示小数点)
 * @param  font   : 字体尺寸
 * @retval 无
 */
void oled_show_float (uint8 x, uint8 y, float num, uint8 int_len, uint8 dec_len, oled_font_enum font)
{
    char buf[16];
    uint8 pos = 0;
    int32_t int_part = (int32_t)num;        /* 取整数部分 */
    float dec_part = num - (float)int_part; /* 取小数部分 */
    if (dec_part < 0) dec_part = -dec_part; /* 处理负数的小数部分 */

    for (uint8 i = 0; i < int_len; i++)
    {
        uint32 div = 1;
        for (uint8 j = 0; j < int_len - i - 1; j++) div *= 10;  /* 计算位权 */
        buf[pos++] = '0' + ((int_part / (int32_t)div) % 10);    /* 整数位转字符 */
    }
    if (dec_len > 0)
    {
        buf[pos++] = '.';  /* 小数点 */
        for (uint8 i = 0; i < dec_len; i++)
        {
            dec_part *= 10;                            /* 逐位左移 */
            buf[pos++] = '0' + ((uint8)dec_part % 10); /* 小数位转字符 */
        }
    }
    buf[pos] = '\0';  /* 字符串结束符 */
    oled_show_string(x, y, buf, font);  /* 显示浮点数字符串 */
}

/**
 * @brief  绘制或清除单个像素点
 * @param  x   : 列坐标
 * @param  y   : 行坐标
 * @param  mode: 1=点亮,0=熄灭
 * @retval 无
 */
void oled_draw_point (uint8 x, uint8 y, uint8 mode)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;  /* 越界判断 */

    uint8 page = y / 8;  /* 计算所在页   */
    uint8 bit  = y % 8;  /* 计算页内位序 */

    if (mode)
        oled_buffer[page][x] |=  (1 << bit);   /* 置位点亮 */
    else
        oled_buffer[page][x] &= ~(1 << bit);   /* 清位熄灭 */
}



/**
 * @brief  OLED 测试函数
 *         初始化 OLED 并在第 2 行显示 "Hello"
 * @retval 无
 */
void oled_test(void){
	i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);  // 总线初始化(应用层负责)
	oled_init();
	oled_show_string(0,  8, "Hello",OLED_FONT_8X16);      // x=0, y=8  (第2行)
	oled_refresh();

}

#endif /* DX_USE_OLED */
