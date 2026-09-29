/**
 * @file    DX_device_as5600.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   AS5600 磁编码器设备头文件
 *          提供 AS5600 角度读取、磁场检测等功能的统一接口
 */

#ifndef __DX_DEVICE_AS5600_H
#define __DX_DEVICE_AS5600_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_AS5600

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_i2c.h"

/* Exported defines ----------------------------------------------------------*/

#define AS5600_ADDR     0x36            /* AS5600 I2C 7 位设备地址 */

/* Exported functions prototypes --------------------------------------------- */

void    as5600_init         (void);                                          /* AS5600 初始化              */
uint16  as5600_read_angle   (void);                                         /* 读取原始角度值 0~4095      */
float   as5600_angle_deg    (void);                                         /* 读取角度值 0.0~360.0°      */
uint8   as5600_is_magnet    (void);                                         /* 磁场检测,1:有磁铁 0:无    */
void    as5600_test         (void);                                         /* AS5600 测试函数            */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_AS5600 */
#endif /* __DX_DEVICE_AS5600_H */
