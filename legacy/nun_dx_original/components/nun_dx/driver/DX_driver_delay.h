/**
 * @file    DX_driver_delay.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   延时驱动头文件
 *          提供基于 SysTick 的微秒级延时及基于 HAL 库的毫秒级延时接口
 */

#ifndef __DX_DRIVER_DELAY_H
#define __DX_DRIVER_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/

void DX_Delay_Init(void);       /* 延时初始化,计算 us 级延时的时钟因子   */
void DX_Delay_ms(uint32_t ms);  /* 毫秒级延时(基于 HAL_Delay)           */
void DX_Delay_us(uint32_t us);  /* 微秒级延时(基于 SysTick 计数)        */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_DELAY_H */
