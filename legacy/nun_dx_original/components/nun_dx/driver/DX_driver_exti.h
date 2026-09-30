/**
 * @file    DX_driver_exti.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   外部中断 (EXTI) 驱动头文件
 *          提供 STM32 EXTI 外部中断的初始化、回调注册等统一接口
 */

#ifndef __DX_DRIVER_EXTI_H
#define __DX_DRIVER_EXTI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief EXTI 触发方式枚举
 *        用于选择外部中断的触发边沿类型
 */
typedef enum
{
    EXTI_TRIG_RISING,   /* 上升沿触发       */
    EXTI_TRIG_FALLING,  /* 下降沿触发       */
    EXTI_TRIG_BOTH,     /* 双边沿触发       */
} exti_trigger_enum;

/**
 * @brief EXTI 中断回调函数类型
 * @param  arg : 用户自定义的回调参数指针
 */
typedef void (*exti_callback_t)(void *arg);

/* Exported functions prototypes ---------------------------------------------*/

void exti_init          (GPIO_TypeDef *port, uint16 pin, exti_trigger_enum trigger);  /* EXTI 初始化并配置中断           */
void exti_set_callback  (GPIO_TypeDef *port, uint16 pin, exti_callback_t cb, void *arg);  /* 注册 EXTI 中断回调函数          */
void exti_irq_handler   (uint16 pin);  /* EXTI 中断分发入口                        */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_EXTI_H */
