/**
 * @file    DX_driver_exti.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   外部中断 (EXTI) 驱动源文件
 *          基于 HAL 库封装 STM32 EXTI 外部中断的初始化、回调及中断服务实现
 */

#include "DX_common_interrupt.h"
#include "DX_driver_gpio.h"
#include "DX_driver_exti.h"

#define EXTI_LINE_COUNT     (16)   /* EXTI 线总数 (0~15) */

/**
 * @brief EXTI 上下文结构体
 *        保存每条 EXTI 线的回调函数、回调参数及初始化标志
 */
typedef struct
{
    exti_callback_t cb;        /* 回调函数指针               */
    void            *arg;     /* 回调函数的用户参数         */
    uint8           init_flag;/* 初始化完成标志             */
} exti_ctx_t;

static exti_ctx_t exti_ctx[EXTI_LINE_COUNT];   /* EXTI 各线上下文数组 */

/**
 * @brief  将 GPIO 引脚位掩码转换为 EXTI 线号
 * @param  pin : GPIO 引脚位掩码 (如 GPIO_PIN_5)
 * @retval 0~15: 对应的 EXTI 线号; 16: 未匹配
 */
static uint32 exti_pin_to_line (uint16 pin)
{
    for (uint32 i = 0; i < 16; i++)
    {
        if (pin & (0x0001U << i)) return i;
    }
    return 16;
}

/**
 * @brief  将 GPIO 端口转换为 AFIO 选择索引
 * @param  port : GPIO 端口 (GPIOA~GPIOE)
 * @retval 0~4: 对应端口索引
 */
static uint32 exti_port_to_index (GPIO_TypeDef *port)
{
    if (port == GPIOA) return 0;
    if (port == GPIOB) return 1;
    if (port == GPIOC) return 2;
    if (port == GPIOD) return 3;
    if (port == GPIOE) return 4;
    return 0;
}

/**
 * @brief  配置 AFIO 的 EXTI 线端口映射
 *         将指定 EXTI 线映射到对应的 GPIO 端口
 * @param  port : GPIO 端口
 * @param  line : EXTI 线号 (0~15)
 * @retval 无
 */
static void exti_afio_config (GPIO_TypeDef *port, uint32 line)
{
    uint32 port_idx = exti_port_to_index(port);
    uint32 reg_idx  = line / 4;
    uint32 pos      = (line % 4) * 4;

    uint32 tmp = AFIO->EXTICR[reg_idx];
    tmp &= ~(0x0FU << pos);
    tmp |= port_idx << pos;
    AFIO->EXTICR[reg_idx] = tmp;
}

/**
 * @brief  EXTI 外部中断初始化
 *         配置 GPIO 为中断输入模式,设置 AFIO 映射并使能对应 NVIC 中断
 * @param  port    : GPIO 端口
 * @param  pin     : GPIO 引脚位掩码
 * @param  trigger : 触发方式,见 exti_trigger_enum
 * @retval 无
 * @note   初始化后默认回调为空,需调用 exti_set_callback 注册回调
 */
void exti_init (GPIO_TypeDef *port, uint16 pin, exti_trigger_enum trigger)
{
    uint32 line = exti_pin_to_line(pin);
    if (line >= EXTI_LINE_COUNT) return;

    gpio_mode_enum mode = DX_IT_RISING;
    if (trigger == EXTI_TRIG_FALLING) mode = DX_IT_FALLING;
    if (trigger == EXTI_TRIG_BOTH)    mode = DX_IT_BOTH;
    DX_GPIO_Init(port, pin, mode);

    __HAL_RCC_AFIO_CLK_ENABLE();
    exti_afio_config(port, line);

    uint32 irq[] =
    {
        EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn,
        EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn, EXTI9_5_IRQn,
        EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn, EXTI15_10_IRQn,
        EXTI15_10_IRQn,
    };

    interrupt_set_priority((IRQn_Type)irq[line], 1, 0);
    interrupt_enable((IRQn_Type)irq[line]);

    exti_ctx[line].cb        = NULL;
    exti_ctx[line].arg       = NULL;
    exti_ctx[line].init_flag = 1;
}

/**
 * @brief  注册 EXTI 中断回调函数
 * @param  port : GPIO 端口
 * @param  pin  : GPIO 引脚位掩码
 * @param  cb   : 回调函数指针
 * @param  arg  : 回调函数的用户参数
 * @retval 无
 */
void exti_set_callback (GPIO_TypeDef *port, uint16 pin, exti_callback_t cb, void *arg)
{
    uint32 line = exti_pin_to_line(pin);
    if (line >= EXTI_LINE_COUNT) return;

    exti_ctx[line].cb  = cb;
    exti_ctx[line].arg = arg;
}

/**
 * @brief  EXTI 中断分发处理
 *         检查中断挂起标志,清除后调用已注册的回调函数
 * @param  pin : GPIO 引脚位掩码
 * @retval 无
 */
static void exti_dispatch (uint16 pin)
{
    uint32 line = exti_pin_to_line(pin);
    if (line >= EXTI_LINE_COUNT) return;

    if (__HAL_GPIO_EXTI_GET_IT(pin) == 0) return;
    __HAL_GPIO_EXTI_CLEAR_IT(pin);

    if (exti_ctx[line].cb)
        exti_ctx[line].cb(exti_ctx[line].arg);
}

/**
 * @brief  EXTI 中断分发入口 (供外部调用)
 * @param  pin : GPIO 引脚位掩码
 * @retval 无
 */
void exti_irq_handler (uint16 pin)
{
    exti_dispatch(pin);
}

#if defined(HAL_GPIO_MODULE_ENABLED)
/**
 * @brief  HAL 库 EXTI 中断回调弱函数重定义
 *         在 HAL GPIO 模块使能时,由 HAL 内部调用,转发到用户回调
 * @param  GPIO_Pin : 触发中断的 GPIO 引脚
 * @retval 无
 */
void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin)
{
    uint32 line = exti_pin_to_line(GPIO_Pin);
    if (line >= EXTI_LINE_COUNT) return;

    if (exti_ctx[line].cb)
        exti_ctx[line].cb(exti_ctx[line].arg);
}
#endif

/* EXTI 线 0~4 中断服务函数 */
#if DX_STANDALONE_IRQ_HANDLERS
void EXTI0_IRQHandler (void)     { exti_dispatch(GPIO_PIN_0); }
void EXTI1_IRQHandler (void)     { exti_dispatch(GPIO_PIN_1); }
void EXTI2_IRQHandler (void)     { exti_dispatch(GPIO_PIN_2); }
void EXTI3_IRQHandler (void)     { exti_dispatch(GPIO_PIN_3); }
void EXTI4_IRQHandler (void)     { exti_dispatch(GPIO_PIN_4); }

/**
 * @brief  EXTI 线 5~9 共用中断服务函数
 *         遍历 5~9 引脚依次进行中断分发
 * @retval 无
 */
void EXTI9_5_IRQHandler (void)
{
    for (uint16 p = GPIO_PIN_5; p <= GPIO_PIN_9; p <<= 1)
        exti_dispatch(p);
}

/**
 * @brief  EXTI 线 10~15 共用中断服务函数
 *         遍历 10~15 引脚依次进行中断分发
 * @retval 无
 */
void EXTI15_10_IRQHandler (void)
{
    for (uint16 p = GPIO_PIN_10; p <= GPIO_PIN_15; p <<= 1)
        exti_dispatch(p);
}
#endif
