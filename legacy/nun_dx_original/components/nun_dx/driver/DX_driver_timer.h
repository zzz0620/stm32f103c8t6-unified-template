/**
 * @file    DX_driver_timer.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   定时器驱动头文件
 *          提供 STM32 定时器(TIM1~TIM4)的初始化、启停、中断及回调等统一接口
 */

#ifndef __DX_DRIVER_TIMER_H
#define __DX_DRIVER_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 定时器索引枚举
 *        用于标识当前使用的定时器实例
 */
typedef enum
{
    TIM_1 = 0,      /* 定时器 1   */
    TIM_2 = 1,      /* 定时器 2   */
    TIM_3 = 2,      /* 定时器 3   */
    TIM_4 = 3,      /* 定时器 4   */
    TIM_COUNT = 4   /* 定时器总数 */
} tim_index_enum;

/**
 * @brief 定时器周期回调函数类型
 * @param  arg: 用户自定义的回调参数指针
 */
typedef void (*tim_callback_t)(void *arg);

/* Exported functions prototypes ---------------------------------------------*/

void    tim_init                (tim_index_enum idx, uint32 prescaler, uint32 period);  /* 定时器初始化            */
void    tim_start               (tim_index_enum idx);                                    /* 启动定时器              */
void    tim_stop                (tim_index_enum idx);                                    /* 停止定时器              */
void    tim_set_callback        (tim_index_enum idx, tim_callback_t cb, void *arg);     /* 设置周期回调函数        */
void    tim_enable_interrupt    (tim_index_enum idx);                                    /* 使能定时器更新中断      */
uint32  tim_get_counter         (tim_index_enum idx);                                    /* 获取当前计数值          */

TIM_HandleTypeDef * tim_get_handle             (tim_index_enum idx);                      /* 获取定时器句柄指针      */
void                tim_invoke_period_callback   (TIM_HandleTypeDef *htim);               /* 调用周期回调(内部使用)  */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_TIMER_H */
