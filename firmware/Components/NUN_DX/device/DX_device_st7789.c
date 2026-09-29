#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_ST7789

/**
 * @file    DX_device_st7789.c
 * @author  YCZ
 * @date    2026-08-04
 * @brief   ST7789 彩色液晶屏驱动源文件
 *          基于 SPI 与 GPIO 驱动实现 240x240 RGB565 彩色屏的初始化与绘图
 */

#include "DX_device_st7789.h"
#include "DX_driver_delay.h"

#define ST7789_BUF_SIZE  (ST7789_WIDTH * 2)  /* 内部发送缓冲区大小(一行像素字节数) */

/**
 * @brief ST7789 硬件运行上下文
 *        保存 SPI 索引及 CS/DC/RST/BL 控制引脚
 */
static struct
{
    spi_index_enum spi;     /* SPI 外设索引   */
    GPIO_TypeDef  *cs_port; /* CS 端口        */
    uint16         cs_pin;  /* CS 引脚        */
    GPIO_TypeDef  *dc_port; /* DC 端口        */
    uint16         dc_pin;  /* DC 引脚        */
    GPIO_TypeDef  *rst_port;/* RST 端口       */
    uint16         rst_pin; /* RST 引脚       */
    GPIO_TypeDef  *bl_port; /* BL 端口        */
    uint16         bl_pin;  /* BL 引脚        */
    uint8          init_flag; /* 初始化完成标志 */
} st7789_ctx;

/* SPI 为全双工,批量发送时需提供接收缓冲区(数据丢弃) */
static uint8 st7789_buf[ST7789_BUF_SIZE];   /* 发送缓冲区    */
static uint8 st7789_rx [ST7789_BUF_SIZE];   /* 虚拟接收缓冲  */

/* -------------------------------------------------------------------------- */
/* 低层硬件操作                                                                */

/**
 * @brief  发送单个 SPI 字节(不管理 CS/DC 电平)
 * @param  dat: 待发送字节
 * @retval 无
 */
static void st7789_spi_byte (uint8 dat)
{
    spi_transfer(st7789_ctx.spi, dat);
}

/**
 * @brief  发送命令(CS 拉低,DC 置命令,发送后 CS 拉高)
 * @param  cmd: 命令字节
 * @retval 无
 */
static void st7789_write_cmd (uint8 cmd)
{
    DX_GPIO_SetLow (st7789_ctx.cs_port, st7789_ctx.cs_pin);   /* 片选拉低     */
    DX_GPIO_SetLow (st7789_ctx.dc_port, st7789_ctx.dc_pin);   /* DC 低 = 命令 */
    st7789_spi_byte(cmd);
    DX_GPIO_SetHigh(st7789_ctx.cs_port, st7789_ctx.cs_pin);   /* 片选拉高     */
}

/**
 * @brief  连续发送数据(CS 拉低,DC 置数据,发送后 CS 拉高)
 * @param  dat: 数据缓冲区
 * @param  len: 发送字节数
 * @retval 无
 */
static void st7789_write_data (const uint8 *dat, uint16 len)
{
    DX_GPIO_SetLow (st7789_ctx.cs_port, st7789_ctx.cs_pin);   /* 片选拉低     */
    DX_GPIO_SetHigh(st7789_ctx.dc_port, st7789_ctx.dc_pin);   /* DC 高 = 数据 */
    spi_transfer_buffer(st7789_ctx.spi, dat, st7789_rx, len); /* 收数据丢弃   */
    DX_GPIO_SetHigh(st7789_ctx.cs_port, st7789_ctx.cs_pin);   /* 片选拉高     */
}

/**
 * @brief  发送单个数据字节(CS 拉低,DC 置数据,发送后 CS 拉高)
 * @param  dat: 数据字节
 * @retval 无
 */
static void st7789_write_data_byte (uint8 dat)
{
    st7789_write_data(&dat, 1);
}

/**
 * @brief  发送一个 16 位数据(RGB565,高字节在前)
 * @param  dat: 16 位数据
 * @retval 无
 */
static void st7789_write_halfword (uint16 dat)
{
    uint8 d[2] = { (uint8)(dat >> 8), (uint8)(dat & 0xFF) };
    st7789_write_data(d, 2);
}

/* -------------------------------------------------------------------------- */
/* 对外接口                                                                    */

/**
 * @brief  初始化 ST7789 屏幕
 *         初始化 SPI 与控制引脚,执行硬件复位及初始化命令序列
 * @param  cfg: 硬件配置指针,见 st7789_cfg_t
 * @retval 无
 */
void st7789_init (const st7789_cfg_t *cfg)
{
    if (cfg == NULL) return;    /* 参数合法性校验 */

    /* 保存硬件配置 */
    st7789_ctx.spi      = cfg->spi;
    st7789_ctx.cs_port  = cfg->cs_port;
    st7789_ctx.cs_pin   = cfg->cs_pin;
    st7789_ctx.dc_port  = cfg->dc_port;
    st7789_ctx.dc_pin   = cfg->dc_pin;
    st7789_ctx.rst_port = cfg->rst_port;
    st7789_ctx.rst_pin  = cfg->rst_pin;
    st7789_ctx.bl_port  = cfg->bl_port;
    st7789_ctx.bl_pin   = cfg->bl_pin;
    st7789_ctx.init_flag = 0;

    /* 初始化 SPI 与控制引脚 */
    spi_init(cfg->spi, cfg->mode, cfg->prescaler, cfg->pin);
    DX_GPIO_Init(cfg->cs_port,  cfg->cs_pin,  DX_OUTPUT_PP);
    DX_GPIO_Init(cfg->dc_port,  cfg->dc_pin,  DX_OUTPUT_PP);
    DX_GPIO_Init(cfg->rst_port, cfg->rst_pin, DX_OUTPUT_PP);
    DX_GPIO_Init(cfg->bl_port,  cfg->bl_pin,  DX_OUTPUT_PP);

    /* 背光点亮,片选默认拉高 */
    DX_GPIO_SetHigh(cfg->bl_port, cfg->bl_pin);
    DX_GPIO_SetHigh(cfg->cs_port, cfg->cs_pin);

    /* 硬件复位时序 */
    DX_GPIO_SetHigh(cfg->rst_port, cfg->rst_pin);
    DX_GPIO_SetLow (cfg->rst_port, cfg->rst_pin);
    DX_Delay_ms(1);
    DX_GPIO_SetHigh(cfg->rst_port, cfg->rst_pin);
    DX_Delay_ms(120);

    /* 初始化命令序列 */
    st7789_write_cmd(0x11);                 /* 退出睡眠                */
    DX_Delay_ms(120);

    st7789_write_cmd(0x3A);                 /* 像素格式                */
    st7789_write_data_byte(0x55);           /* 16bit RGB565            */

    st7789_write_cmd(0xB2);                 /* Porch 设置              */
    st7789_write_data_byte(0x0C);
    st7789_write_data_byte(0x0C);
    st7789_write_data_byte(0x00);
    st7789_write_data_byte(0x33);
    st7789_write_data_byte(0x33);

    st7789_write_cmd(0xB7);                 /* 栅极控制                */
    st7789_write_data_byte(0x35);

    st7789_write_cmd(0xBB);                 /* VCOM 设置               */
    st7789_write_data_byte(0x32);

    st7789_write_cmd(0xC2);                 /* 电源控制 2              */
    st7789_write_data_byte(0x01);

    st7789_write_cmd(0xC3);                 /* 电源控制 3              */
    st7789_write_data_byte(0x19);

    st7789_write_cmd(0xC4);                 /* 电源控制 4              */
    st7789_write_data_byte(0x20);

    st7789_write_cmd(0xC6);                 /* 帧率控制                */
    st7789_write_data_byte(0x0F);

    st7789_write_cmd(0xD0);                 /* 电源控制 1              */
    st7789_write_data_byte(0xA4);
    st7789_write_data_byte(0xA1);

    /* 正伽马校准 */
    st7789_write_cmd(0xE0);
    st7789_write_data_byte(0xD0);
    st7789_write_data_byte(0x08);
    st7789_write_data_byte(0x0E);
    st7789_write_data_byte(0x09);
    st7789_write_data_byte(0x09);
    st7789_write_data_byte(0x05);
    st7789_write_data_byte(0x31);
    st7789_write_data_byte(0x33);
    st7789_write_data_byte(0x48);
    st7789_write_data_byte(0x17);
    st7789_write_data_byte(0x14);
    st7789_write_data_byte(0x15);
    st7789_write_data_byte(0x31);
    st7789_write_data_byte(0x34);

    /* 负伽马校准 */
    st7789_write_cmd(0xE1);
    st7789_write_data_byte(0xD0);
    st7789_write_data_byte(0x08);
    st7789_write_data_byte(0x0E);
    st7789_write_data_byte(0x09);
    st7789_write_data_byte(0x09);
    st7789_write_data_byte(0x15);
    st7789_write_data_byte(0x31);
    st7789_write_data_byte(0x33);
    st7789_write_data_byte(0x48);
    st7789_write_data_byte(0x17);
    st7789_write_data_byte(0x14);
    st7789_write_data_byte(0x15);
    st7789_write_data_byte(0x31);
    st7789_write_data_byte(0x34);

    st7789_write_cmd(0x21);                 /* 打开反显                */

    st7789_write_cmd(0x36);                 /* 内存访问控制            */
    st7789_write_data_byte(0x00);           /* 默认方向(可改为 0x60/0xC0/0xA0 旋转) */

    st7789_write_cmd(0x2A);                 /* 列地址设置              */
    st7789_write_halfword(0x0000);          /* 列起始 0               */
    st7789_write_halfword(0x00EF);          /* 列结束 239             */

    st7789_write_cmd(0x2B);                 /* 行地址设置              */
    st7789_write_halfword(0x0000);          /* 行起始 0               */
    st7789_write_halfword(0x00EF);          /* 行结束 239             */

    st7789_write_cmd(0x29);                 /* 打开显示                */

    st7789_ctx.init_flag = 1;               /* 标记初始化完成          */
}

/**
 * @brief  设置刷屏窗口
 *         指定后续绘图操作的列/行地址范围
 * @param  x1: 列起始地址
 * @param  y1: 行起始地址
 * @param  x2: 列结束地址
 * @param  y2: 行结束地址
 * @retval 无
 */
void st7789_set_window (uint16 x1, uint16 y1, uint16 x2, uint16 y2)
{
    st7789_write_cmd(0x2A);                 /* 列地址设置 */
    st7789_write_halfword(x1);
    st7789_write_halfword(x2);
    st7789_write_cmd(0x2B);                 /* 行地址设置 */
    st7789_write_halfword(y1);
    st7789_write_halfword(y2);
    st7789_write_cmd(0x2C);                 /* 存储器写   */
}

/**
 * @brief  清屏(以指定颜色填充整屏)
 * @param  color: RGB565 颜色
 * @retval 无
 */
void st7789_clear (uint16 color)
{
    st7789_set_window(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    for (uint16 i = 0; i < ST7789_BUF_SIZE; i += 2)   /* 填充颜色到发送缓冲 */
    {
        st7789_buf[i]     = color >> 8;
        st7789_buf[i + 1] = color & 0xFF;
    }

    uint32 total  = (uint32)ST7789_WIDTH * ST7789_HEIGHT * 2;
    uint32 count  = total / ST7789_BUF_SIZE;
    uint32 remain = total % ST7789_BUF_SIZE;

    for (uint32 i = 0; i < count; i++)
        st7789_write_data(st7789_buf, ST7789_BUF_SIZE);
    if (remain)
        st7789_write_data(st7789_buf, remain);
}

/**
 * @brief  绘制单个像素
 * @param  x    : 列坐标
 * @param  y    : 行坐标
 * @param  color: RGB565 颜色
 * @retval 无
 */
void st7789_draw_pixel (uint16 x, uint16 y, uint16 color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;   /* 越界保护 */
    st7789_set_window(x, y, x, y);
    st7789_write_halfword(color);
}

/**
 * @brief  填充矩形区域
 * @param  x1   : 列起始
 * @param  y1   : 行起始
 * @param  x2   : 列结束
 * @param  y2   : 行结束
 * @param  color: RGB565 颜色
 * @retval 无
 */
void st7789_fill_rect (uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 color)
{
    uint32 w     = x2 - x1 + 1;
    uint32 h     = y2 - y1 + 1;
    uint32 total = w * h * 2;

    st7789_set_window(x1, y1, x2, y2);

    for (uint16 i = 0; i < ST7789_BUF_SIZE; i += 2)   /* 填充颜色到发送缓冲 */
    {
        st7789_buf[i]     = color >> 8;
        st7789_buf[i + 1] = color & 0xFF;
    }

    uint32 count  = total / ST7789_BUF_SIZE;
    uint32 remain = total % ST7789_BUF_SIZE;

    for (uint32 i = 0; i < count; i++)
        st7789_write_data(st7789_buf, ST7789_BUF_SIZE);
    if (remain)
        st7789_write_data(st7789_buf, remain);
}

/**
 * @brief  绘制位图(逐行发送)
 * @param  x     : 列起始
 * @param  y     : 行起始
 * @param  width : 位图宽度(像素)
 * @param  height: 位图高度(像素)
 * @param  data  : 位图数据(RGB565,高字节在前,按行排列)
 * @retval 无
 */
void st7789_draw_bitmap (uint16 x, uint16 y, uint16 width, uint16 height, const uint8 *data)
{
    uint16 line_bytes = width * 2;      /* 单行字节数 */

    for (uint16 row = 0; row < height; row++)
    {
        st7789_set_window(x, y + row, x + width - 1, y + row);
        st7789_write_data(data + (uint32)row * line_bytes, line_bytes);
    }
}

/**
 * @brief  显示单个字符
 *         使用工程字库 ascii_font_6x8 / ascii_font_8x16 渲染到屏幕
 * @param  x      : 列坐标
 * @param  y      : 行坐标
 * @param  ch     : 待显示字符
 * @param  font   : 字体尺寸,见 st7789_font_enum
 * @param  color  : 前景色(RGB565)
 * @param  bgcolor: 背景色(RGB565)
 * @retval 无
 * @note   字模为列行式(每列一个字节,bit0=最上行)
 */
void st7789_show_char (uint16 x, uint16 y, char ch, st7789_font_enum font, uint16 color, uint16 bgcolor)
{
    uint8 idx = (uint8)ch - 32;         /* ASCII 偏移(跳过前 32 个控制字符) */
    uint8 w, h;
    const uint8 *p;

    if (font == ST7789_FONT_6X8) { w = 6; h = 8;  p = ascii_font_6x8[idx]; }
    else                          { w = 8; h = 16; p = ascii_font_8x16[idx]; }

    st7789_set_window(x, y, x + w - 1, y + h - 1);

    /* CS 保持低、DC 保持高,连续输出像素数据 */
    DX_GPIO_SetLow (st7789_ctx.cs_port, st7789_ctx.cs_pin);
    DX_GPIO_SetHigh(st7789_ctx.dc_port, st7789_ctx.dc_pin);

    for (uint8 r = 0; r < h; r++)
    {
        uint8 band = r / 8;             /* 8x16 字体分为上下两个字节块 */
        uint8 bit  = r % 8;             /* 字节内行号                  */
        for (uint8 c = 0; c < w; c++)
        {
            uint8  byte = p[band * w + c];
            uint16 col  = (byte & (1 << bit)) ? color : bgcolor;   /* 判断像素是否点亮 */
            st7789_spi_byte(col >> 8);
            st7789_spi_byte(col & 0xFF);
        }
    }

    DX_GPIO_SetHigh(st7789_ctx.cs_port, st7789_ctx.cs_pin);
}

/**
 * @brief  显示字符串
 *         逐字符显示,超出屏宽自动换行,超出屏高停止
 * @param  x      : 起始列坐标
 * @param  y      : 起始行坐标
 * @param  str    : 待显示字符串
 * @param  font   : 字体尺寸,见 st7789_font_enum
 * @param  color  : 前景色(RGB565)
 * @param  bgcolor: 背景色(RGB565)
 * @retval 无
 */
void st7789_show_string (uint16 x, uint16 y, const char *str, st7789_font_enum font, uint16 color, uint16 bgcolor)
{
    uint8 w, h;
    if (font == ST7789_FONT_6X8) { w = 6; h = 8; }
    else                          { w = 8; h = 16; }

    while (*str)
    {
        st7789_show_char(x, y, *str++, font, color, bgcolor);
        x += w;
        if (x + w > ST7789_WIDTH) { x = 0; y += h; }   /* 超出屏宽换行 */
        if (y + h > ST7789_HEIGHT) break;              /* 超出屏高退出 */
    }
}

/**
 * @brief  ST7789 测试函数
 *         清屏为黑色并显示一行白色字符串
 * @retval 无
 * @note   调用前需先执行 st7789_init 完成初始化
 */
void st7789_test (void)
{
    st7789_clear(RGB565_BLACK);     /* 清屏黑色 */
    st7789_show_string(0, 0, "Hello ST7789", ST7789_FONT_8X16, RGB565_WHITE, RGB565_BLACK);
}

/* --------------------------------- 使用示例 -------------------------------- */
/*
    // 1. 定义硬件配置(引脚需与实际接线一致,SPI 使用模式 0)
    st7789_cfg_t lcd_cfg = {
        .spi       = SPI_1,
        .mode      = SPI_MODE_0,
        .prescaler = SPI_BAUDRATEPRESCALER_2,
        .pin       = SPI1_SCK_PA5_MISO_PA6_MOSI_PA7,
        .cs_port   = GPIOB, .cs_pin  = GPIO_PIN_6,   // CS
        .dc_port   = GPIOB, .dc_pin  = GPIO_PIN_7,   // DC
        .rst_port  = GPIOB, .rst_pin = GPIO_PIN_8,   // RST
        .bl_port   = GPIOB, .bl_pin  = GPIO_PIN_9,   // BL
    };

    // 2. 初始化并显示
    st7789_init(&lcd_cfg);
    st7789_clear(RGB565_WHITE);
    st7789_show_string(0, 0, "Hello ST7789", ST7789_FONT_8X16, RGB565_RED, RGB565_WHITE);
*/

#endif /* DX_USE_ST7789 */
