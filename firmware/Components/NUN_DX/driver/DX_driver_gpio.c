/**
 * @file    DX_driver_gpio.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   GPIO 驱动源文件
 *          基于 HAL 库封装 GPIO 初始化、读写、翻转等操作的实现
 */

#include "DX_driver_gpio.h"

/**
 * @brief  GPIO 初始化
 * @param  GPIOx   : GPIO 端口 (如 GPIOA、GPIOB、GPIOC 等)
 * @param  GPIO_Pin: 引脚号 (如 GPIO_PIN_13)
 * @param  mode    : GPIO 工作模式,见 gpio_mode_enum
 * @retval 无
 */
void DX_GPIO_Init (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN, gpio_mode_enum mode)
{
    GPIO_InitTypeDef cfg = {0};

    /* 根据端口使能对应的 GPIO 时钟 */
    if (GPIOx == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (GPIOx == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (GPIOx == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (GPIOx == GPIOD)  __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (GPIOx == GPIOE)  __HAL_RCC_GPIOE_CLK_ENABLE();

    cfg.Pin  = GPIO_PIN;
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;   /* 默认高速 */

    /* 根据模式配置 GPIO 工作方式及上下拉 */
    switch (mode)
    {
        case DX_OUTPUT_PP:     cfg.Mode = GPIO_MODE_OUTPUT_PP;      cfg.Pull = GPIO_NOPULL;   break;  /* 推挽输出       */
        case DX_OUTPUT_OD:     cfg.Mode = GPIO_MODE_OUTPUT_OD;      cfg.Pull = GPIO_NOPULL;   break;  /* 开漏输出       */
        case DX_INPUT_FLOAT:   cfg.Mode = GPIO_MODE_INPUT;          cfg.Pull = GPIO_NOPULL;   break;  /* 浮空输入       */
        case DX_INPUT_PULLUP:  cfg.Mode = GPIO_MODE_INPUT;          cfg.Pull = GPIO_PULLUP;   break;  /* 上拉输入       */
        case DX_INPUT_PULLDOWN:cfg.Mode = GPIO_MODE_INPUT;          cfg.Pull = GPIO_PULLDOWN; break;  /* 下拉输入       */
        case DX_ANALOG:        cfg.Mode = GPIO_MODE_ANALOG;         cfg.Pull = GPIO_NOPULL;   break;  /* 模拟模式       */
        case DX_AF_PP:         cfg.Mode = GPIO_MODE_AF_PP;          cfg.Pull = GPIO_NOPULL;   break;  /* 复用推挽输出   */
        case DX_AF_OD:         cfg.Mode = GPIO_MODE_AF_OD;          cfg.Pull = GPIO_NOPULL;   break;  /* 复用开漏输出   */
        case DX_IT_RISING:     cfg.Mode = GPIO_MODE_IT_RISING;      cfg.Pull = GPIO_PULLUP;   break;  /* 上升沿中断     */
        case DX_IT_FALLING:    cfg.Mode = GPIO_MODE_IT_FALLING;     cfg.Pull = GPIO_PULLUP;   break;  /* 下降沿中断     */
        case DX_IT_BOTH:       cfg.Mode = GPIO_MODE_IT_RISING_FALLING; cfg.Pull = GPIO_PULLUP; break; /* 双边沿中断     */
    }

    HAL_GPIO_Init(GPIOx, &cfg);
}

/**
 * @brief  设置 GPIO 输出高电平
 * @param  GPIOx   : GPIO 端口
 * @param  GPIO_Pin: 引脚号
 * @retval 无
 */
void DX_GPIO_SetHigh (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief  设置 GPIO 输出低电平
 * @param  GPIOx   : GPIO 端口
 * @param  GPIO_Pin: 引脚号
 * @retval 无
 */
void DX_GPIO_SetLow (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  翻转 GPIO 输出电平
 * @param  GPIOx   : GPIO 端口
 * @param  GPIO_Pin: 引脚号
 * @retval 无
 */
void DX_GPIO_Toggle (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN)
{
    HAL_GPIO_TogglePin(GPIOx, GPIO_PIN);
}

/**
 * @brief  读取 GPIO 输入电平
 * @param  GPIOx   : GPIO 端口
 * @param  GPIO_Pin: 引脚号
 * @retval 1: 高电平; 0: 低电平
 */
uint8 DX_GPIO_Read (GPIO_TypeDef *GPIOx, uint16_t GPIO_PIN)
{
    return (HAL_GPIO_ReadPin(GPIOx, GPIO_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

/**
 * @brief  GPIO 测试函数
 *         以 PA0 为例,循环翻转实现 LED 闪烁测试,周期 500ms
 * @retval 无
 */
void DX_GPIO_Test (void)
{
    DX_GPIO_Init(GPIOA, GPIO_PIN_0, DX_OUTPUT_PP);

    while (1)
    {
        DX_GPIO_Toggle(GPIOA, GPIO_PIN_0);
        DX_Delay_ms(500);
    }
}
