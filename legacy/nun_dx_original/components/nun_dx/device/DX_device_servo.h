/**
 * @file    DX_device_servo.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   舵机设备头文件
 *          提供基于 PWM 的舵机角度控制接口(50Hz / 0~180°)
 */

#ifndef __DX_DEVICE_SERVO_H
#define __DX_DEVICE_SERVO_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_SERVO

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_pwm.h"

/* Exported defines ----------------------------------------------------------*/

#define SERVO_FREQ_HZ       50      /* 舵机控制频率 50Hz (周期 20ms) */
#define SERVO_DUTY_MIN      250     /* 0°   对应 0.5ms 高电平        */
#define SERVO_DUTY_MAX      1250    /* 180° 对应 2.5ms 高电平        */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 舵机引脚映射枚举
 *        列举各定时器通道对应的舵机控制引脚
 */
typedef enum
{
    SERVO_TIM1_CH1_PA8,     /* TIM1 通道1 PA8  */
    SERVO_TIM1_CH2_PA9,     /* TIM1 通道2 PA9  */
    SERVO_TIM1_CH3_PA10,    /* TIM1 通道3 PA10 */
    SERVO_TIM1_CH4_PA11,    /* TIM1 通道4 PA11 */

    SERVO_TIM2_CH1_PA0,     /* TIM2 通道1 PA0  */
    SERVO_TIM2_CH2_PA1,     /* TIM2 通道2 PA1  */
    SERVO_TIM2_CH3_PA2,     /* TIM2 通道3 PA2  */
    SERVO_TIM2_CH4_PA3,     /* TIM2 通道4 PA3  */

    SERVO_TIM3_CH1_PA6,     /* TIM3 通道1 PA6  */
    SERVO_TIM3_CH2_PA7,     /* TIM3 通道2 PA7  */
    SERVO_TIM3_CH3_PB0,     /* TIM3 通道3 PB0  */
    SERVO_TIM3_CH4_PB1,     /* TIM3 通道4 PB1  */
    SERVO_TIM3_CH1_PB4,     /* TIM3 通道1 PB4  */
    SERVO_TIM3_CH2_PB5,     /* TIM3 通道2 PB5  */
    SERVO_TIM3_CH3_PC8,     /* TIM3 通道3 PC8  */
    SERVO_TIM3_CH4_PC9,     /* TIM3 通道4 PC9  */

    SERVO_TIM4_CH1_PB6,     /* TIM4 通道1 PB6  */
    SERVO_TIM4_CH2_PB7,     /* TIM4 通道2 PB7  */
    SERVO_TIM4_CH3_PB8,     /* TIM4 通道3 PB8  */
    SERVO_TIM4_CH4_PB9,     /* TIM4 通道4 PB9  */
} servo_pin_enum;

/* Exported functions prototypes ---------------------------------------------*/

void servo_init       (servo_pin_enum pin);              /* 初始化舵机对应 PWM 通道   */
void servo_set_angle  (servo_pin_enum pin, uint8 angle); /* 设置舵机角度 (0~180°)    */
void servo_test       (void);                            /* 舵机测试函数             */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_SERVO */
#endif /* __DX_DEVICE_SERVO_H */
