/**
 * @file    DX_device_aht10.h
 * @author  YCZ
 * @date    2026-08-02
 * @brief   AHT10 温湿度传感器设备头文件
 *          提供基于 I2C 的温湿度测量接口
 */

#ifndef __DX_DEVICE_AHT10_H
#define __DX_DEVICE_AHT10_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_AHT10

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_i2c.h"

/* Exported defines ----------------------------------------------------------*/

#define AHT10_ADDR      0x38    /* AHT10 I2C 7 位设备地址 */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief AHT10 温湿度数据结构体
 *        存放一次测量得到的温度(℃)与相对湿度(%)
 */
typedef struct
{
    float temp;     /* 温度,单位 ℃   */
    float hum;      /* 相对湿度,单位 % */
} aht10_data_t;

/* Exported functions prototypes ---------------------------------------------*/

void    aht10_init      (void);             /* AHT10 初始化(校准命令)   */
uint8   aht10_read      (aht10_data_t *data); /* 触发测量并读取温湿度    */
void    aht10_test      (void);             /* AHT10 测试函数           */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_AHT10 */
#endif /* __DX_DEVICE_AHT10_H */
