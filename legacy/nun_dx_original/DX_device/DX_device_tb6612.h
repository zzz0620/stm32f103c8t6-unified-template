/**
 * @file    DX_device_tb6612.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   TB6612 电机驱动头文件
 *          提供 TB6612 双 H 桥电机驱动的初始化、方向控制、PWM 调速等统一接口
 */

#ifndef __DX_DEVICE_TB6612_H
#define __DX_DEVICE_TB6612_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_TB6612

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_headfile.h"
#include "DX_driver_pwm.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief TB6612 电机通道枚举
 *        对应 8 路 PWM 输出,每路独立控制一个直流电机
 */
typedef enum
{
    TB6612_M1_TIM1_CH1,     /* 电机1,PWM: PA8 */
    TB6612_M2_TIM1_CH2,     /* 电机2,PWM: PA9 */
    TB6612_M3_TIM2_CH1,     /* 电机3,PWM: PA0 */
    TB6612_M4_TIM2_CH2,     /* 电机4,PWM: PA1 */
    TB6612_M5_TIM3_CH1,     /* 电机5,PWM: PA6 */
    TB6612_M6_TIM3_CH2,     /* 电机6,PWM: PA7 */
    TB6612_M7_TIM4_CH1,     /* 电机7,PWM: PB6 */
    TB6612_M8_TIM4_CH2,     /* 电机8,PWM: PB7 */
} tb6612_motor_enum;

/* Exported functions prototypes ---------------------------------------------*/

void tb6612_init     (tb6612_motor_enum motor,                              /* 电机初始化            */
                      GPIO_TypeDef *in1_port, uint16 in1_pin,
                      GPIO_TypeDef *in2_port, uint16 in2_pin);
void tb6612_set_stby (GPIO_TypeDef *port, uint16 pin);                       /* 配置 STBY 待机引脚    */
void tb6612_enable   (void);                                                  /* 使能芯片(退出待机)    */
void tb6612_disable  (void);                                                  /* 禁用芯片(进入待机)    */
void tb6612_forward  (tb6612_motor_enum motor);                               /* 电机正转              */
void tb6612_backward (tb6612_motor_enum motor);                               /* 电机反转              */
void tb6612_brake    (tb6612_motor_enum motor);                               /* 电机制动(短接制动)    */
void tb6612_stop     (tb6612_motor_enum motor);                               /* 电机滑行(自由停止)    */
void tb6612_set_speed(tb6612_motor_enum motor, uint32 speed);                 /* 设置占空比 0~10000    */

#ifdef __cplusplus
}
#endif

#endif /* DX_USE_TB6612 */
#endif /* __DX_DEVICE_TB6612_H */
