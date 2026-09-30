/**
 * @file    DX_driver_pwm.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   PWM 驱动源文件
 *          基于 HAL 库封装定时器 PWM 输出的初始化、占空比设置与启停等实现
 */

#include "DX_driver_pwm.h"

/**
 * @brief PWM 通道运行上下文结构体
 *        保存定时器句柄、通道、周期及初始化标志,供占空比设置与启停使用
 */
typedef struct
{
    TIM_HandleTypeDef *htim;        /* 定时器句柄              */
    uint32             channel;     /* 通道号                  */
    uint32             period;      /* 自动重装载值(arr)       */
    uint8              init_flag;   /* 初始化完成标志          */
    uint8              started;     /* 输出启动标志(0:未启动)  */
} pwm_ctx_t;

static pwm_ctx_t pwm_ctx[TIM_COUNT][4];  /* 各定时器各通道的运行上下文 */

/**
 * @brief PWM 引脚配置结构体
 *        描述某路 PWM 引脚对应的端口、引脚号、定时器索引、通道及重映射参数
 */
typedef struct
{
    GPIO_TypeDef *port;        /* GPIO 端口        */
    uint16        pin;         /* GPIO 引脚号      */
    tim_index_enum tim_idx;    /* 定时器索引       */
    uint32        channel;     /* 定时器通道       */
    uint32        remap_val;   /* 重映射配置值     */
    uint32        remap_mask;  /* 重映射掩码       */
} pwm_pin_config_t;

/* PWM 引脚映射表:枚举值 -> 端口/引脚/定时器/通道/重映射参数 */
static const pwm_pin_config_t pwm_pin_table[] =
{
    [PWM_TIM1_CH1_PA8]  = { GPIOA, GPIO_PIN_8,  TIM_1, TIM_CHANNEL_1, 0x00000000, AFIO_MAPR_TIM1_REMAP_Msk },
    [PWM_TIM1_CH2_PA9]  = { GPIOA, GPIO_PIN_9,  TIM_1, TIM_CHANNEL_2, 0x00000000, AFIO_MAPR_TIM1_REMAP_Msk },
    [PWM_TIM1_CH3_PA10] = { GPIOA, GPIO_PIN_10, TIM_1, TIM_CHANNEL_3, 0x00000000, AFIO_MAPR_TIM1_REMAP_Msk },
    [PWM_TIM1_CH4_PA11] = { GPIOA, GPIO_PIN_11, TIM_1, TIM_CHANNEL_4, 0x00000000, AFIO_MAPR_TIM1_REMAP_Msk },
    [PWM_TIM2_CH1_PA0]  = { GPIOA, GPIO_PIN_0,  TIM_2, TIM_CHANNEL_1, 0, 0 },
    [PWM_TIM2_CH2_PA1]  = { GPIOA, GPIO_PIN_1,  TIM_2, TIM_CHANNEL_2, 0, 0 },
    [PWM_TIM2_CH3_PA2]  = { GPIOA, GPIO_PIN_2,  TIM_2, TIM_CHANNEL_3, 0, 0 },
    [PWM_TIM2_CH4_PA3]  = { GPIOA, GPIO_PIN_3,  TIM_2, TIM_CHANNEL_4, 0, 0 },

    [PWM_TIM3_CH1_PA6]  = { GPIOA, GPIO_PIN_6,  TIM_3, TIM_CHANNEL_1, 0x00000000, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH2_PA7]  = { GPIOA, GPIO_PIN_7,  TIM_3, TIM_CHANNEL_2, 0x00000000, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH3_PB0]  = { GPIOB, GPIO_PIN_0,  TIM_3, TIM_CHANNEL_3, 0x00000000, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH4_PB1]  = { GPIOB, GPIO_PIN_1,  TIM_3, TIM_CHANNEL_4, 0x00000000, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH1_PB4]  = { GPIOB, GPIO_PIN_4,  TIM_3, TIM_CHANNEL_1, AFIO_MAPR_TIM3_REMAP_PARTIALREMAP, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH2_PB5]  = { GPIOB, GPIO_PIN_5,  TIM_3, TIM_CHANNEL_2, AFIO_MAPR_TIM3_REMAP_PARTIALREMAP, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH3_PC8]  = { GPIOC, GPIO_PIN_8,  TIM_3, TIM_CHANNEL_3, AFIO_MAPR_TIM3_REMAP_FULLREMAP, AFIO_MAPR_TIM3_REMAP_Msk },
    [PWM_TIM3_CH4_PC9]  = { GPIOC, GPIO_PIN_9,  TIM_3, TIM_CHANNEL_4, AFIO_MAPR_TIM3_REMAP_FULLREMAP, AFIO_MAPR_TIM3_REMAP_Msk },

    [PWM_TIM4_CH1_PB6]  = { GPIOB, GPIO_PIN_6,  TIM_4, TIM_CHANNEL_1, 0, 0 },
    [PWM_TIM4_CH2_PB7]  = { GPIOB, GPIO_PIN_7,  TIM_4, TIM_CHANNEL_2, 0, 0 },
    [PWM_TIM4_CH3_PB8]  = { GPIOB, GPIO_PIN_8,  TIM_4, TIM_CHANNEL_3, 0, 0 },
    [PWM_TIM4_CH4_PB9]  = { GPIOB, GPIO_PIN_9,  TIM_4, TIM_CHANNEL_4, 0, 0 },
};

/* 定时器实例数组(PWM 初始化自包含,不依赖 tim_init 预先设置句柄 Instance) */
static TIM_TypeDef *const pwm_tim_instance[] =
{
    TIM1, TIM2, TIM3, TIM4
};

/**
 * @brief  使能 GPIO 端口时钟
 * @param  port: GPIO 端口 (如 GPIOA、GPIOB 等)
 * @retval 无
 */
static void pwm_gpio_clk_enable (GPIO_TypeDef *port)
{
    if (port == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD)  __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE)  __HAL_RCC_GPIOE_CLK_ENABLE();
}

#define TIM_CLK_HZ  72000000UL  /* 定时器时钟频率(Hz) */

/**
 * @brief  PWM 初始化
 *         根据指定频率配置定时器时基与输出比较通道,完成 GPIO 复用配置与引脚重映射,
 *         并将运行上下文保存至 pwm_ctx 供后续占空比设置与启停使用
 * @param  idx    : 定时器索引,见 tim_index_enum
 * @param  ch     : PWM 通道,见 pwm_channel_enum
 * @param  freq_hz: PWM 输出频率(Hz)
 * @param  pin    : PWM 引脚配置,见 pwm_pin_enum
 * @retval 无
 * @note   占空比初始为 0,需调用 pwm_set_duty 与 pwm_start 后方可输出
 */
void pwm_init (tim_index_enum idx, pwm_channel_enum ch, uint32 freq_hz, pwm_pin_enum pin)
{
    if (idx >= TIM_COUNT || freq_hz == 0) return;  /* 参数合法性校验 */

    const pwm_pin_config_t *pincfg = &pwm_pin_table[pin];

    __HAL_RCC_AFIO_CLK_ENABLE();          /* 使能 AFIO 时钟(重映射所需) */
    pwm_gpio_clk_enable(pincfg->port);    /* 使能对应 GPIO 端口时钟 */

    MODIFY_REG(AFIO->MAPR, pincfg->remap_mask, pincfg->remap_val);  /* 配置引脚重映射 */

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = pincfg->pin;
    gpio.Mode  = GPIO_MODE_AF_PP;         /* 复用推挽输出 */
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;    /* 高速 */
    HAL_GPIO_Init(pincfg->port, &gpio);

    TIM_HandleTypeDef *htim = tim_get_handle(idx);
    if (NULL == htim) return;             /* 获取定时器句柄失败则退出 */

    htim->Instance = pwm_tim_instance[pincfg->tim_idx];  /* 指定定时器实例(自包含) */

    if (htim->Instance == TIM1) __HAL_RCC_TIM1_CLK_ENABLE();  /* 使能对应定时器时钟 */
    if (htim->Instance == TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
    if (htim->Instance == TIM3) __HAL_RCC_TIM3_CLK_ENABLE();
    if (htim->Instance == TIM4) __HAL_RCC_TIM4_CLK_ENABLE();

    uint32 total_div = TIM_CLK_HZ / freq_hz;  /* 总分频系数 */
    uint32 psc, arr;

    if (total_div <= 65536)
    {
        psc = 0;                         /* 分频系数较小时,不分频 */
        arr = total_div - 1;
    }
    else
    {
        psc = (total_div - 1) / 65536;   /* 需预分频以满足 16 位计数范围 */
        arr = (TIM_CLK_HZ / ((psc + 1) * freq_hz)) - 1;
    }

    htim->Init.Prescaler         = psc;
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;            /* 向上计数 */
    htim->Init.Period            = arr;
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;        /* 时钟不分频 */
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /* 使能 ARR 预装载 */
    HAL_TIM_PWM_Init(htim);

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode     = TIM_OCMODE_PWM1;       /* PWM 模式 1 */
    oc.Pulse      = 0;                      /* 初始占空比为 0 */
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;    /* 高电平有效 */
    oc.OCFastMode = TIM_OCFAST_DISABLE;     /* 关闭快速模式 */
    HAL_TIM_PWM_ConfigChannel(htim, &oc, ch);

    pwm_ctx[idx][ch - TIM_CHANNEL_1].htim      = htim;    /* 保存运行上下文 */
    pwm_ctx[idx][ch - TIM_CHANNEL_1].channel   = ch;
    pwm_ctx[idx][ch - TIM_CHANNEL_1].period    = arr;
    pwm_ctx[idx][ch - TIM_CHANNEL_1].init_flag = 1;
}

/**
 * @brief  设置 PWM 占空比
 * @param  idx : 定时器索引,见 tim_index_enum
 * @param  ch  : PWM 通道,见 pwm_channel_enum
 * @param  duty: 占空比,范围 0-10000(对应 0%-100%)
 * @retval 无
 */
void pwm_set_duty (tim_index_enum idx, pwm_channel_enum ch, uint32 duty)
{
    if (idx >= TIM_COUNT || (ch - TIM_CHANNEL_1) >= 4) return;  /* 参数合法性校验 */
    pwm_ctx_t *ctx = &pwm_ctx[idx][ch - TIM_CHANNEL_1];
    if (!ctx->init_flag) return;                                /* 未初始化则退出 */

    if (duty > 10000) duty = 10000;                            /* 限幅处理 */
    uint32 pulse = (duty * (ctx->period + 1)) / 10000;         /* 换算为比较寄存器值 */
    __HAL_TIM_SET_COMPARE(ctx->htim, ctx->channel, pulse);     /* 更新比较值 */

    if (!ctx->started)                                          /* 未启动则自动启动输出 */
    {
        HAL_TIM_PWM_Start(ctx->htim, ctx->channel);
        ctx->started = 1;
    }
}

/**
 * @brief  启动 PWM 输出
 * @param  idx: 定时器索引,见 tim_index_enum
 * @param  ch : PWM 通道,见 pwm_channel_enum
 * @retval 无
 */
void pwm_start (tim_index_enum idx, pwm_channel_enum ch)
{
    if (idx >= TIM_COUNT || (ch - TIM_CHANNEL_1) >= 4) return;  /* 参数合法性校验 */
    pwm_ctx_t *ctx = &pwm_ctx[idx][ch - TIM_CHANNEL_1];
    if (!ctx->init_flag) return;                                /* 未初始化则退出 */
    HAL_TIM_PWM_Start(ctx->htim, ctx->channel);                 /* 启动 PWM 输出 */
    ctx->started = 1;                                           /* 标记已启动 */
}

/**
 * @brief  停止 PWM 输出
 * @param  idx: 定时器索引,见 tim_index_enum
 * @param  ch : PWM 通道,见 pwm_channel_enum
 * @retval 无
 */
void pwm_stop (tim_index_enum idx, pwm_channel_enum ch)
{
    if (idx >= TIM_COUNT || (ch - TIM_CHANNEL_1) >= 4) return;  /* 参数合法性校验 */
    pwm_ctx_t *ctx = &pwm_ctx[idx][ch - TIM_CHANNEL_1];
    if (!ctx->init_flag) return;                                /* 未初始化则退出 */
    HAL_TIM_PWM_Stop(ctx->htim, ctx->channel);                  /* 停止 PWM 输出 */
    ctx->started = 0;                                           /* 标记已停止 */
}
