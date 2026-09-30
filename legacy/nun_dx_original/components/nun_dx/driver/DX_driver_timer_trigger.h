/**
 * @file    DX_driver_timer_trigger.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   定时器触发驱动头文件
 *          基于通用定时器封装周期性触发接口,支持 Hz/ms/us 三种时间单位
 */

#ifndef __DX_DRIVER_TIMER_TRIGGER_H
#define __DX_DRIVER_TIMER_TRIGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_driver_timer.h"


// 冲突保护：默认 TIM 留给 PWM,不接管 ISR
// 若某 TIM 需用于定时触发,在 include 前 #define 对应宏即可启用其 ISR
				
//  #define DX_TIM1_TRIGGER_ENABLE
  #define DX_TIM2_TRIGGER_ENABLE
//  #define DX_TIM3_TRIGGER_ENABLE
//  #define DX_TIM4_TRIGGER_ENABLE


/* Exported types ------------------------------------------------------------*/

/**
 * @brief 定时器触发时间单位枚举
 *        用于指定触发周期以频率(Hz)或时间(ms/us)表示
 */
typedef enum
{
    TIM_UNIT_HZ,    /* 频率 (Hz)          */
    TIM_UNIT_MS,    /* 周期 (ms)          */
    TIM_UNIT_US,    /* 周期 (us)          */
} tim_unit_enum;

/* 用户实现的回调 -------------------------------------------------------------*/

extern void tim1_trigger_callback (void);  /* TIM1 触发回调,需由用户实现 */
extern void tim2_trigger_callback (void);  /* TIM2 触发回调,需由用户实现 */
extern void tim3_trigger_callback (void);  /* TIM3 触发回调,需由用户实现 */
extern void tim4_trigger_callback (void);  /* TIM4 触发回调,需由用户实现 */

/* Exported functions prototypes ---------------------------------------------*/

void tim_trigger_start (tim_index_enum idx, uint32 value, tim_unit_enum unit);  /* 启动定时器周期触发    */
void tim_trigger_stop  (tim_index_enum idx);                                    /* 停止定时器触发        */
void tim_trigger_test  (void);                                                  /* 定时器触发测试        */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_TIMER_TRIGGER_H */
