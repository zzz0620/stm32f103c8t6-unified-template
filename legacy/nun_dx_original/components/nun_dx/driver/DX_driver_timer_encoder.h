/**
 * @file    DX_driver_timer_encoder.h
 * @author  YCZ
 * @date    2026-08-29
 * @brief   定时器编码器驱动头文件
 *          基于 STM32 定时器编码器模式封装 A/B 正交编码器计数接口
 */

#ifndef __DX_DRIVER_TIMER_ENCODER_H
#define __DX_DRIVER_TIMER_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_driver_timer.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 编码器计数模式枚举
 *        对应定时器编码器接口的三种计数方式
 */
typedef enum
{
    ENC_MODE_TI1,   /* 单倍频:仅 A 相计数 */
    ENC_MODE_TI2,   /* 单倍频:仅 B 相计数 */
    ENC_MODE_TI12,  /* 四倍频:A/B 正交计数(推荐) */
} enc_mode_enum;

/**
 * @brief 编码器引脚枚举
 *        列举支持的编码器引脚配置(A 相接 CH1,B 相接 CH2)
 */
typedef enum
{
    ENC_TIM1_CH1_PA8,   /* TIM1 CH1/CH2 = PA8/PA9   */
    ENC_TIM2_CH1_PA0,   /* TIM2 CH1/CH2 = PA0/PA1(32位计数器) */
    ENC_TIM3_CH1_PA6,   /* TIM3 CH1/CH2 = PA6/PA7   */
    ENC_TIM3_CH1_PB4,   /* TIM3 CH1/CH2 = PB4/PB5(部分重映射) */
    ENC_TIM4_CH1_PB6,   /* TIM4 CH1/CH2 = PB6/PB7   */
} enc_pin_enum;

/* Exported functions prototypes ---------------------------------------------*/

void  enc_init      (tim_index_enum idx, enc_pin_enum pin, enc_mode_enum mode);  /* 编码器初始化                */
void  enc_start     (tim_index_enum idx);                                         /* 启动编码器计数              */
void  enc_stop      (tim_index_enum idx);                                         /* 停止编码器计数              */
int32 enc_get_count (tim_index_enum idx);                                         /* 获取累加有符号位置(含方向)  */
uint32 enc_get_raw  (tim_index_enum idx);                                         /* 获取硬件原始计数值          */
void  enc_clear     (tim_index_enum idx);                                         /* 位置清零                    */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_TIMER_ENCODER_H */
