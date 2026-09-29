/**
 * @file    DX_config.h
 * @author  YCZ
 * @date    2026-08-24
 * @brief   工程配置开关头文件
 *          集中管理各设备模块的启用/禁用开关:
 *          置 0 的模块 .c 整体被 #if 剔除,不编译、不占 Flash,
 *          统一头文件也不再包含其 API,避免误用
 */



#ifndef __DX_CONFIG_H
#define __DX_CONFIG_H

/* 设备层开关:1=启用,0=禁用 ------------------------------------------------ */
#define DX_USE_OLED        0   /* 0.9 寸 OLED 屏(I2C)          */
#define DX_USE_AHT10        0   /* AHT10 温湿度传感器(I2C)      */
#define DX_USE_AT24C64        0   /* AT24C64 EEPROM(I2C)          */
#define DX_USE_AS5600        0   /* AS5600 磁编码器(I2C)         */
#define DX_USE_BH1750        0   /* BH1750 光照强度传感器(I2C)   */
#define DX_USE_MPU6050        0   /* MPU6050 六轴姿态传感器(I2C)  */
#define DX_USE_KEY        1   /* 按键(GPIO,软件消抖)          */
#define DX_USE_SERVO        0   /* 舵机(PWM)                   */
#define DX_USE_ST7789        0   /* ST7789 彩屏(SPI)             */
#define DX_USE_TB6612        0   /* TB6612 双 H 桥电机驱动       */
#define DX_USE_ZDT_EMM_V5        0   /* 张大头 Emm_V5.0 闭环步进驱动(UART2) */




/* 说明:DX_driver 驱动层保持无条件编译,其占用的 Flash 仅在对应
   设备启用时通过函数引用被链接,不用的外设驱动不会增加最终镜像 */

#endif /* __DX_CONFIG_H */
