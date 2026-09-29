/**
 * @file    DX_driver_pwm.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   PWM 驱动头文件
 *          基于 STM32 定时器封装 PWM 输出接口,支持多通道及引脚重映射
 */

#ifndef __DX_DRIVER_PWM_H
#define __DX_DRIVER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_driver_timer.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief PWM 通道枚举
 *        对应定时器的 4 个捕获/比较通道
 */
typedef enum
{
    PWM_CH_1 = TIM_CHANNEL_1,   /* 通道 1 */
    PWM_CH_2 = TIM_CHANNEL_2,   /* 通道 2 */
    PWM_CH_3 = TIM_CHANNEL_3,   /* 通道 3 */
    PWM_CH_4 = TIM_CHANNEL_4,   /* 通道 4 */
} pwm_channel_enum;

/**
 * @brief PWM 引脚枚举
 *        列举支持的 PWM 引脚配置,涵盖定时器、通道与引脚映射
 */
typedef enum
{
    PWM_TIM1_CH1_PA8,   /* TIM1 通道1 PA8  */
    PWM_TIM1_CH2_PA9,   /* TIM1 通道2 PA9  */
    PWM_TIM1_CH3_PA10,  /* TIM1 通道3 PA10 */
    PWM_TIM1_CH4_PA11,  /* TIM1 通道4 PA11 */

    PWM_TIM2_CH1_PA0,   /* TIM2 通道1 PA0 */
    PWM_TIM2_CH2_PA1,   /* TIM2 通道2 PA1 */
    PWM_TIM2_CH3_PA2,   /* TIM2 通道3 PA2 */
    PWM_TIM2_CH4_PA3,   /* TIM2 通道4 PA3 */

    PWM_TIM3_CH1_PA6,   /* TIM3 通道1 PA6            */
    PWM_TIM3_CH2_PA7,   /* TIM3 通道2 PA7            */
    PWM_TIM3_CH3_PB0,   /* TIM3 通道3 PB0            */
    PWM_TIM3_CH4_PB1,   /* TIM3 通道4 PB1            */
    PWM_TIM3_CH1_PB4,   /* TIM3 通道1 PB4(部分重映射) */
    PWM_TIM3_CH2_PB5,   /* TIM3 通道2 PB5(部分重映射) */
    PWM_TIM3_CH3_PC8,   /* TIM3 通道3 PC8(完全重映射) */
    PWM_TIM3_CH4_PC9,   /* TIM3 通道4 PC9(完全重映射) */

    PWM_TIM4_CH1_PB6,   /* TIM4 通道1 PB6 */
    PWM_TIM4_CH2_PB7,   /* TIM4 通道2 PB7 */
    PWM_TIM4_CH3_PB8,   /* TIM4 通道3 PB8 */
    PWM_TIM4_CH4_PB9,   /* TIM4 通道4 PB9 */
} pwm_pin_enum;

/* Exported functions prototypes ---------------------------------------------*/

void pwm_init     (tim_index_enum idx, pwm_channel_enum ch, uint32 freq_hz, pwm_pin_enum pin);  /* PWM 初始化                   */
void pwm_set_duty (tim_index_enum idx, pwm_channel_enum ch, uint32 duty);                       /* 设置占空比 (duty: 0-10000),未启动时自动启动输出 */
void pwm_start    (tim_index_enum idx, pwm_channel_enum ch);                                    /* 启动 PWM 输出                */
void pwm_stop     (tim_index_enum idx, pwm_channel_enum ch);                                    /* 停止 PWM 输出                */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_PWM_H */
