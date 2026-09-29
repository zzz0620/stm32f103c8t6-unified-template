#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_AS5600

/**
 * @file    DX_device_as5600.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   AS5600 磁编码器设备源文件
 *          基于 I2C 驱动封装 AS5600 角度读取、磁场检测等功能的实现
 */

#include "DX_device_as5600.h"
#include "DX_driver_delay.h"

#define AS5600_REG_ANGLE            0x0E                /* 角度寄存器地址(已滤波) */
#define AS5600_REG_RAW_ANGLE        0x0C                /* 原始角度寄存器地址      */
#define AS5600_REG_STATUS           0x0B                /* 状态寄存器地址          */

/* AS5600 设备句柄:总线、地址、超时一处定义,总线由应用层 i2c_init 初始化 */
static const i2c_dev_t as5600_dev = { I2C_1, AS5600_ADDR, 100 };

/**
 * @brief  AS5600 初始化
 *         无需设备级配置,总线初始化由应用层负责
 * @retval 无
 */
void as5600_init (void)
{
}

/**
 * @brief  读取 AS5600 角度值
 *         从角度寄存器读取 12 位角度原始值
 * @retval 0~4095 对应 0°~360°
 */
uint16 as5600_read_angle (void)
{
    uint8 buf[2];
    i2c_dev_mem_read(&as5600_dev, AS5600_REG_ANGLE, buf, 2);   /* 读取 2 字节角度数据 */
    return ((uint16)buf[0] << 8) | buf[1];                              /* 拼接为 16 位数据     */
}

/**
 * @brief  读取 AS5600 角度(角度制)
 *         将原始 12 位角度值转换为角度制
 * @retval 0.0~360.0°
 */
float as5600_angle_deg (void)
{
    return as5600_read_angle() * (360.0f / 4096.0f);   /* 12 位分辨率映射到 0~360° */
}

/**
 * @brief  检测磁铁是否存在
 *         读取状态寄存器的 MH 位(bit5)判断磁铁磁场强度是否在有效范围
 * @retval 1: 磁铁存在; 0: 磁铁不存在
 */
uint8 as5600_is_magnet (void)
{
    uint8 status;
    i2c_dev_mem_read(&as5600_dev, AS5600_REG_STATUS, &status, 1);   /* 读取状态寄存器 */
    return (status & 0x20) ? 1 : 0;                                        /* bit5 为 MH 位,1 表示磁铁存在 */
}

/**
 * @brief  AS5600 测试函数
 *         循环读取角度值和磁场状态并通过串口打印,周期 200ms
 * @retval 无
 */
void as5600_test (void)
{
    i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);   /* 总线初始化(应用层负责) */
    as5600_init();                                                         /* 初始化 AS5600 */

    while (1)
    {
        printf("Angle: %.2f  Mag: %d\r\n",
               as5600_angle_deg(),
               as5600_is_magnet());                                        /* 打印角度值与磁场状态 */

        DX_Delay_ms(200);                                                  /* 延时 200ms */
    }
}

#endif /* DX_USE_AS5600 */
