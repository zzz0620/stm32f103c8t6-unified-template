/**
 * @file    DX_device_i2c_mpu6050.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   MPU6050 六轴传感器驱动头文件
 *          基于 I2C 封装 MPU6050 的初始化、ID 读取、陀螺仪/加速度计/温度
 *          数据读取及量程转换等接口
 */

#ifndef __DX_DEVICE_I2C_MPU6050_H
#define __DX_DEVICE_I2C_MPU6050_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_MPU6050

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_i2c.h"

/* Exported defines ----------------------------------------------------------*/

#define MPU6050_ADDR    0x68    /* MPU6050 的 I2C 设备地址(7 位)    */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief MPU6050 三轴原始数据结构体
 *        用于存放陀螺仪或加速度计 X/Y/Z 轴的 16 位有符号原始值
 */
typedef struct
{
    int16 x, y, z;
} mpu6050_raw_data_t;

/* Exported functions prototypes ---------------------------------------------*/

void    mpu6050_init        (i2c_index_enum i2c_idx);                     /* MPU6050 初始化(选择总线) */
uint8   mpu6050_get_id      (void);                                        /* 读取 WHO_AM_I 设备 ID     */
void    mpu6050_read_gyro   (mpu6050_raw_data_t *data);                   /* 读取陀螺仪三轴原始数据    */
void    mpu6050_read_accel  (mpu6050_raw_data_t *data);                   /* 读取加速度计三轴原始数据  */
int16   mpu6050_read_temp   (void);                                        /* 读取温度原始数据          */
float   mpu6050_gyro_dps    (int16 raw);                                   /* 陀螺仪原始值转换为 °/s    */
float   mpu6050_accel_g     (int16 raw);                                   /* 加速度计原始值转换为 g    */
void    mpu6050_test         (void);                                       /* MPU6050 测试函数          */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_MPU6050 */
#endif /* __DX_DEVICE_I2C_MPU6050_H */
