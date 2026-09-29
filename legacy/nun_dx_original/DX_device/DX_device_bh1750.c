#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_BH1750

/**
 * @file    DX_device_bh1750.c
 * @author  YCZ
 * @date    2026-08-02
 * @brief   BH1750 光照强度传感器设备源文件
 *          基于 I2C 驱动实现 BH1750 光照强度读取(单次 H 分辨率模式)
 */

#include "DX_device_bh1750.h"
#include "DX_driver_delay.h"

#define BH1750_CMD_POWER_ON     0x01    /* 上电命令              */
#define BH1750_CMD_H_RES_ONCE   0x20    /* 单次 H 分辨率测量命令 */
#define BH1750_H_RES_MEAS_TIME  180     /* H 分辨率单次测量时间(ms) */

/* BH1750 设备句柄:总线、地址、超时一处定义,总线由应用层 i2c_init 初始化 */
static const i2c_dev_t bh1750_dev = { I2C_1, BH1750_ADDR, 100 };

/**
 * @brief  读取 BH1750 光照强度
 *         发送单次 H 分辨率测量命令,等待测量完成后读取 2 字节原始值
 * @retval 光照强度,单位 lx
 * @note   H 分辨率模式 1lx/bit,原始值除以 1.2 即 lux
 */
float bh1750_read_lux (void)
{
    uint8 cmd = BH1750_CMD_H_RES_ONCE;
    uint8 buf[2];

    if (i2c_dev_write(&bh1750_dev, &cmd, 1) != 0) return -1.0f;
    DX_Delay_ms(BH1750_H_RES_MEAS_TIME);          /* 等待测量完成 */

    if (i2c_dev_read(&bh1750_dev, buf, 2) != 0) return -1.0f;

    uint16 raw = ((uint16)buf[0] << 8) | buf[1];  /* 2 字节原始值高字节在前 */
    return raw / 1.2f;
}

/**
 * @brief  BH1750 测试函数
 *         循环读取光照强度并通过串口打印,周期 500ms
 * @retval 无
 */
void bh1750_test (void)
{
    i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);  /* 总线初始化(应用层负责) */

    uint8 cmd = BH1750_CMD_POWER_ON;
    i2c_dev_write(&bh1750_dev, &cmd, 1);            /* 上电 */

    while (1)
    {
        printf("Lux: %.1f\r\n", bh1750_read_lux()); /* 打印光照强度 */
        DX_Delay_ms(500);
    }
}

#endif /* DX_USE_BH1750 */
