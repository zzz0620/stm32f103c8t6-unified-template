/**
 * @file    DX_driver_gpio.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   GPIO 驱动头文件
 *          提供 STM32 GPIO 的初始化、读写、翻转等操作的统一接口
 */

#ifndef __DX_DRIVER_GPIO_H
#define __DX_DRIVER_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"
#include "DX_driver_delay.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief GPIO 工作模式枚举
 *        对 HAL 库 GPIO 模式进行封装,便于统一调用
 */
typedef enum
{
    DX_OUTPUT_PP,       /* 推挽输出           */
    DX_OUTPUT_OD,       /* 开漏输出           */
    DX_INPUT_FLOAT,     /* 浮空输入           */
    DX_INPUT_PULLUP,    /* 上拉输入           */
    DX_INPUT_PULLDOWN,  /* 下拉输入           */
    DX_ANALOG,          /* 模拟模式           */
    DX_AF_PP,           /* 复用推挽输出       */
    DX_AF_OD,           /* 复用开漏输出       */
    DX_IT_RISING,       /* 上升沿触发中断     */
    DX_IT_FALLING,      /* 下降沿触发中断     */
    DX_IT_BOTH,         /* 双边沿触发中断     */
} gpio_mode_enum;

/* Exported functions prototypes ---------------------------------------------*/

void    DX_GPIO_Init    (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN, gpio_mode_enum mode);  /* GPIO 初始化         */
void    DX_GPIO_SetHigh (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN);                       /* 输出高电平          */
void    DX_GPIO_SetLow  (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN);                       /* 输出低电平          */
void    DX_GPIO_Toggle  (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN);                       /* 翻转输出电平        */
uint8   DX_GPIO_Read    (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN);                       /* 读取输入电平        */

void    DX_GPIO_Test    (void);                                                         /* GPIO 测试函数       */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_GPIO_H */
