/**
 * @file    DX_driver_i2c.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   I2C 驱动头文件
 *          分层设计:总线级(i2c_init)由应用层初始化,设备级(i2c_dev_*)
 *          通过设备句柄绑定总线、地址与超时,支持同一总线挂载多个设备
 */

#ifndef __DX_DRIVER_I2C_H
#define __DX_DRIVER_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief I2C 外设实例索引枚举
 *        用于选择使用的 I2C 实例
 */
typedef enum
{
    I2C_1 = 0,      /* I2C1 实例            */
    I2C_2 = 1,      /* I2C2 实例            */
    I2C_COUNT = 2   /* I2C 实例总数         */
} i2c_index_enum;

/**
 * @brief I2C 引脚映射枚举
 *        列举 I2C 引脚的可用复用组合
 */
typedef enum
{
    I2C1_SCL_PB6_SDA_PB7,     /* I2C1 : SCL=PB6,  SDA=PB7   */
    I2C1_SCL_PB8_SDA_PB9,     /* I2C1 : SCL=PB8,  SDA=PB9   */
    I2C2_SCL_PB10_SDA_PB11,   /* I2C2 : SCL=PB10, SDA=PB11  */
} i2c_pin_enum;

/**
 * @brief I2C 设备句柄
 *        绑定设备所在总线、7 位地址与单次传输超时,
 *        同一总线上多个设备各持一个句柄,换总线/改地址只改句柄一处
 */
typedef struct
{
    i2c_index_enum bus;     /* 所在 I2C 总线实例   */
    uint8          addr;    /* 7 位设备地址        */
    uint32         timeout; /* 单次传输超时 (ms)   */
} i2c_dev_t;

/* Exported functions prototypes ---------------------------------------------*/

void    i2c_init           (i2c_index_enum idx, uint32 speed, i2c_pin_enum pin);  /* 初始化 I2C 外设及引脚复用(总线级,应用层调用)  */
void    i2c_attach         (i2c_index_enum idx, I2C_HandleTypeDef *handle);        /* 绑定 CubeMX/板级代码已经初始化的 HAL 句柄 */
uint8   i2c_dev_write      (const i2c_dev_t *dev, const uint8 *buf, uint16 len);  /* 向设备写入数据(无寄存器地址)   */
uint8   i2c_dev_read       (const i2c_dev_t *dev, uint8 *buf, uint16 len);  /* 从设备读取数据(无寄存器地址)   */
uint8   i2c_dev_mem_write  (const i2c_dev_t *dev, uint8 reg, const uint8 *buf, uint16 len);  /* 向设备寄存器写入数据    */
uint8   i2c_dev_mem_read   (const i2c_dev_t *dev, uint8 reg, uint8 *buf, uint16 len);  /* 从设备寄存器读取数据    */
uint8   i2c_dev_mem_write16 (const i2c_dev_t *dev, uint16 reg, const uint8 *buf, uint16 len);  /* 向设备16位寄存器地址写入数据 */
uint8   i2c_dev_mem_read16  (const i2c_dev_t *dev, uint16 reg, uint8 *buf, uint16 len);  /* 从设备16位寄存器地址读取数据 */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_I2C_H */
