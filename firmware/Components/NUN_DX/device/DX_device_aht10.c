#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_AHT10

/**
 * @file    DX_device_aht10.c
 * @author  YCZ
 * @date    2026-08-02
 * @brief   AHT10 温湿度传感器设备源文件
 *          基于 I2C 驱动实现 AHT10 的初始化、测量触发与温湿度数据解析
 */

#include "DX_device_aht10.h"
#include "DX_driver_delay.h"

#define AHT10_CMD_INIT          0xE1    /* 初始化/校准命令    */
#define AHT10_CMD_INIT_DATA1    0x08    /* 初始化数据字节 1   */
#define AHT10_CMD_INIT_DATA2    0x00    /* 初始化数据字节 2   */
#define AHT10_CMD_MEASURE       0xAC    /* 触发测量命令       */
#define AHT10_CMD_MEASURE_DATA1 0x33    /* 测量命令数据字节 1 */
#define AHT10_CMD_MEASURE_DATA2 0x00    /* 测量命令数据字节 2 */
#define AHT10_MEASURE_TIME      80      /* 测量时间(ms)       */

/* AHT10 设备句柄:总线、地址、超时一处定义,总线由应用层 i2c_init 初始化 */
static const i2c_dev_t aht10_dev = { TEMPLATE_DX_I2C_BUS, AHT10_ADDR, 100 };

/**
 * @brief  AHT10 初始化
 *         发送初始化/校准命令,使传感器进入可测量状态
 * @retval 无
 */
void aht10_init (void)
{
    uint8 cmd[3] = { AHT10_CMD_INIT, AHT10_CMD_INIT_DATA1, AHT10_CMD_INIT_DATA2 };
    i2c_dev_write(&aht10_dev, cmd, 3);
    DX_Delay_ms(20);                      /* 等待校准完成 */
}

/**
 * @brief  触发一次测量并读取温湿度
 *         发送测量命令,等待测量完成后读取 6 字节数据并解析
 * @param  data: 指向存放温湿度数据的结构体指针
 * @retval 0: 成功; 1: 失败
 * @note   数据格式:status(1) + 湿度 20bit + 温度 20bit,
 *         湿度 = raw*100/2^20,温度 = raw*200/2^20 - 50
 */
uint8 aht10_read (aht10_data_t *data)
{
    uint8 cmd[3] = { AHT10_CMD_MEASURE, AHT10_CMD_MEASURE_DATA1, AHT10_CMD_MEASURE_DATA2 };
    uint8 buf[6];

    if (i2c_dev_write(&aht10_dev, cmd, 3) != 0) return 1;
    DX_Delay_ms(AHT10_MEASURE_TIME);      /* 等待测量完成 */

    if (i2c_dev_read(&aht10_dev, buf, 6) != 0) return 1;

    uint32 hum_raw  = ((uint32)buf[1] << 12) | ((uint32)buf[2] << 4) | (buf[3] >> 4);
    uint32 temp_raw = (((uint32)buf[3] & 0x0F) << 16) | ((uint32)buf[4] << 8) | buf[5];

    data->hum  = hum_raw  * 100.0f / (1UL << 20);
    data->temp = temp_raw * 200.0f / (1UL << 20) - 50.0f;
    return 0;
}

/**
 * @brief  AHT10 测试函数
 *         初始化后循环读取温湿度并通过串口打印,周期 1s
 * @retval 无
 */
void aht10_test (void)
{
    i2c_init(I2C_1, 400000, I2C1_SCL_PB8_SDA_PB9);  /* 总线初始化(应用层负责) */
    aht10_init();

    while (1)
    {
        aht10_data_t data;
        if (aht10_read(&data) == 0)
            printf("Temp: %.1f C  Hum: %.1f%%\r\n", data.temp, data.hum);  /* 打印温湿度 */
        DX_Delay_ms(1000);
    }
}

#endif /* DX_USE_AHT10 */
