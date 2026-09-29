/**
 * @file    DX_driver_timer.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   定时器驱动源文件
 *          基于 HAL 库封装 TIM1~TIM4 的初始化、启停、中断及回调管理等实现
 */

#include "DX_common_interrupt.h"
#include "DX_driver_timer.h"

/**
 * @brief 定时器上下文结构体
 *        保存每个定时器的句柄、初始化标志及回调信息
 */
typedef struct
{
    TIM_HandleTypeDef handle;       /* HAL 定时器句柄              */
    uint8             init_flag;    /* 初始化完成标志:1 已初始化   */

    tim_callback_t    period_cb;    /* 周期溢出回调函数指针        */
    void              *cb_arg;      /* 回调函数用户参数            */
} tim_ctx_t;

static tim_ctx_t tim_ctx[TIM_COUNT];  /* 各定时器的上下文数组 */

static TIM_TypeDef *const tim_instance[] =
{
    TIM1, TIM2, TIM3, TIM4
};

/**
 * @brief  定时器初始化
 * @param  idx       : 定时器索引,见 tim_index_enum
 * @param  prescaler : 预分频值
 * @param  period    : 自动重装载值(周期)
 * @retval 无
 * @note   配置为向上计数模式,使能自动重装载预装载
 */
void tim_init (tim_index_enum idx, uint32 prescaler, uint32 period)
{
    if (idx >= TIM_COUNT) return;                       /* 索引越界直接返回 */

    tim_ctx_t *ctx = &tim_ctx[idx];
    ctx->init_flag = 0;                                 /* 初始化前置标志为 0 */

    if (idx == TIM_1) __HAL_RCC_TIM1_CLK_ENABLE();      /* 使能 TIM1 时钟 */
    if (idx == TIM_2) __HAL_RCC_TIM2_CLK_ENABLE();      /* 使能 TIM2 时钟 */
    if (idx == TIM_3) __HAL_RCC_TIM3_CLK_ENABLE();      /* 使能 TIM3 时钟 */
    if (idx == TIM_4) __HAL_RCC_TIM4_CLK_ENABLE();      /* 使能 TIM4 时钟 */

    TIM_HandleTypeDef *htim = &ctx->handle;
    htim->Instance               = tim_instance[idx];
    htim->Init.Prescaler         = prescaler;           /* 预分频值 */
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;  /* 向上计数模式 */
    htim->Init.Period            = period;              /* 自动重装载周期 */
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;        /* 时钟不分频 */
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /* 使能 ARPE */
    HAL_TIM_Base_Init(htim);

    ctx->init_flag = 1;                                 /* 标记初始化完成 */
}

/**
 * @brief  启动定时器
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void tim_start (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    tim_ctx_t *ctx = &tim_ctx[idx];
    if (!ctx->init_flag) return;                        /* 未初始化则返回 */
    HAL_TIM_Base_Start(&ctx->handle);
}

/**
 * @brief  停止定时器
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void tim_stop (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    tim_ctx_t *ctx = &tim_ctx[idx];
    if (!ctx->init_flag) return;                        /* 未初始化则返回 */
    HAL_TIM_Base_Stop(&ctx->handle);
}

/**
 * @brief  设置定时器周期回调函数
 * @param  idx: 定时器索引,见 tim_index_enum
 * @param  cb : 回调函数指针
 * @param  arg: 传递给回调函数的用户参数
 * @retval 无
 */
void tim_set_callback (tim_index_enum idx, tim_callback_t cb, void *arg)
{
    if (idx >= TIM_COUNT) return;
    tim_ctx_t *ctx = &tim_ctx[idx];
    ctx->period_cb = cb;                                /* 保存回调函数 */
    ctx->cb_arg    = arg;                               /* 保存回调参数 */
}

/**
 * @brief  使能定时器更新中断
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void tim_enable_interrupt (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    tim_ctx_t *ctx = &tim_ctx[idx];
    if (!ctx->init_flag) return;                        /* 未初始化则返回 */

    uint32 irq[] = { TIM1_UP_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn };  /* 各定时器对应的中断号 */
    __HAL_TIM_ENABLE_IT(&ctx->handle, TIM_IT_UPDATE);   /* 使能更新中断 */
    interrupt_set_priority((IRQn_Type)irq[idx], 1, 0);  /* 设置中断优先级 */
    interrupt_enable((IRQn_Type)irq[idx]);              /* 使能 NVIC 中断 */
}

/**
 * @brief  获取定时器当前计数值
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 当前计数值;未初始化或越界时返回 0
 */
uint32 tim_get_counter (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return 0;
    tim_ctx_t *ctx = &tim_ctx[idx];
    if (!ctx->init_flag) return 0;                      /* 未初始化则返回 0 */
    return __HAL_TIM_GET_COUNTER(&ctx->handle);
}

/**
 * @brief  获取定时器句柄指针
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 定时器句柄指针;越界时返回 NULL
 */
TIM_HandleTypeDef * tim_get_handle (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return NULL;
    return &tim_ctx[idx].handle;
}

/**
 * @brief  调用定时器周期回调函数
 *         根据传入的句柄匹配对应的定时器索引,并触发其注册的回调
 * @param  htim: 定时器句柄指针
 * @retval 无
 * @note   供中断服务或 HAL 回调内部调用
 */
void tim_invoke_period_callback (TIM_HandleTypeDef *htim)
{
    tim_index_enum idx;
    if (htim->Instance == TIM1) idx = TIM_1;            /* 匹配 TIM1 */
    else if (htim->Instance == TIM2) idx = TIM_2;       /* 匹配 TIM2 */
    else if (htim->Instance == TIM3) idx = TIM_3;       /* 匹配 TIM3 */
    else if (htim->Instance == TIM4) idx = TIM_4;       /* 匹配 TIM4 */
    else return;                                        /* 不支持的实例则返回 */

    tim_ctx_t *ctx = &tim_ctx[idx];
    if (ctx->period_cb)
        ctx->period_cb(ctx->cb_arg);                    /* 调用用户回调 */
}
