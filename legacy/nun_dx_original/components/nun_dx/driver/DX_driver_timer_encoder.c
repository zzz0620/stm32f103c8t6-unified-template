/**
 * @file    DX_driver_timer_encoder.c
 * @author  YCZ
 * @date    2026-08-29
 * @brief   定时器编码器驱动源文件
 *          基于 HAL 库封装定时器编码器模式的初始化、启停与计数值读取等实现
 */

#include "DX_driver_timer_encoder.h"

/**
 * @brief 编码器运行上下文结构体
 *        保存定时器句柄、初始化标志及软件累加位置信息
 */
typedef struct
{
    TIM_HandleTypeDef *htim;        /* 定时器句柄              */
    uint8              init_flag;   /* 初始化完成标志:1 已初始化 */
    uint32             last_raw;    /* 上次读取的原始计数值    */
    int32              position;    /* 累加有符号位置(含方向)  */
    uint32             wrap_mask;   /* 计数器回绕掩码(0xFFFF 或 0xFFFFFFFF) */
} enc_ctx_t;

static enc_ctx_t enc_ctx[TIM_COUNT];  /* 各定时器的编码器上下文数组 */

/**
 * @brief 编码器引脚配置结构体
 *        描述某组编码器引脚对应的端口、A/B 相引脚号、定时器索引及重映射参数
 */
typedef struct
{
    GPIO_TypeDef *port;        /* GPIO 端口        */
    uint16        pin1;        /* A 相引脚号(CH1)  */
    uint16        pin2;        /* B 相引脚号(CH2)  */
    tim_index_enum tim_idx;    /* 定时器索引       */
    uint32        remap_val;   /* 重映射配置值     */
    uint32        remap_mask;  /* 重映射掩码       */
} enc_pin_config_t;

/* 编码器引脚映射表:枚举值 -> 端口/A相/B相引脚/定时器/重映射参数 */
static const enc_pin_config_t enc_pin_table[] =
{
    [ENC_TIM1_CH1_PA8] = { GPIOA, GPIO_PIN_8,  GPIO_PIN_9,  TIM_1, 0x00000000, AFIO_MAPR_TIM1_REMAP_Msk },
    [ENC_TIM2_CH1_PA0] = { GPIOA, GPIO_PIN_0,  GPIO_PIN_1,  TIM_2, 0, 0 },
    [ENC_TIM3_CH1_PA6] = { GPIOA, GPIO_PIN_6,  GPIO_PIN_7,  TIM_3, 0x00000000, AFIO_MAPR_TIM3_REMAP_Msk },
    [ENC_TIM3_CH1_PB4] = { GPIOB, GPIO_PIN_4,  GPIO_PIN_5,  TIM_3, AFIO_MAPR_TIM3_REMAP_PARTIALREMAP, AFIO_MAPR_TIM3_REMAP_Msk },
    [ENC_TIM4_CH1_PB6] = { GPIOB, GPIO_PIN_6,  GPIO_PIN_7,  TIM_4, 0, 0 },
};

/* 定时器实例数组(编码器初始化自包含,不依赖 tim_init 预先设置句柄 Instance) */
static TIM_TypeDef *const enc_tim_instance[] =
{
    TIM1, TIM2, TIM3, TIM4
};

/**
 * @brief  使能 GPIO 端口时钟
 * @param  port: GPIO 端口 (如 GPIOA、GPIOB 等)
 * @retval 无
 */
static void enc_gpio_clk_enable (GPIO_TypeDef *port)
{
    if (port == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD)  __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE)  __HAL_RCC_GPIOE_CLK_ENABLE();
}

/**
 * @brief  编码器初始化
 *         配置编码器输入引脚(复用浮空输入)与定时器编码器模式,
 *         计数器范围 TIM2 为 32 位,其余为 16 位
 * @param  idx : 定时器索引,见 tim_index_enum
 * @param  pin : 编码器引脚配置,见 enc_pin_enum
 * @param  mode: 计数模式,见 enc_mode_enum
 * @retval 无
 * @note   编码器模式为硬件计数,无需中断;计数值通过 enc_get_count 读取
 */
void enc_init (tim_index_enum idx, enc_pin_enum pin, enc_mode_enum mode)
{
    if (idx >= TIM_COUNT) return;             /* 索引越界直接返回 */

    enc_ctx_t *ctx = &enc_ctx[idx];
    const enc_pin_config_t *pincfg = &enc_pin_table[pin];

    __HAL_RCC_AFIO_CLK_ENABLE();              /* 使能 AFIO 时钟(重映射所需) */
    enc_gpio_clk_enable(pincfg->port);        /* 使能对应 GPIO 端口时钟 */

    MODIFY_REG(AFIO->MAPR, pincfg->remap_mask, pincfg->remap_val);  /* 配置引脚重映射 */

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = pincfg->pin1 | pincfg->pin2; /* A/B 两相引脚 */
    gpio.Mode  = GPIO_MODE_AF_INPUT;          /* 复用输入 */
    gpio.Pull  = GPIO_PULLUP;                 /* 内部上拉(兼容开漏输出的编码器) */
    HAL_GPIO_Init(pincfg->port, &gpio);

    TIM_HandleTypeDef *htim = tim_get_handle(idx);
    if (NULL == htim) return;                 /* 获取定时器句柄失败则退出 */

    htim->Instance = enc_tim_instance[idx];   /* 指定定时器实例(自包含) */

    if (htim->Instance == TIM1) __HAL_RCC_TIM1_CLK_ENABLE();  /* 使能对应定时器时钟 */
    if (htim->Instance == TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
    if (htim->Instance == TIM3) __HAL_RCC_TIM3_CLK_ENABLE();
    if (htim->Instance == TIM4) __HAL_RCC_TIM4_CLK_ENABLE();

    uint8 is_32bit = (htim->Instance == TIM2);                /* TIM2 为 32 位计数器 */

    htim->Init.Prescaler         = 0;                         /* 编码器模式不使用预分频 */
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;        /* 向上计数 */
    htim->Init.Period            = is_32bit ? 0xFFFFFFFF : 0xFFFF;  /* 计数范围按位宽设置 */
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;    /* 时钟不分频 */
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    TIM_Encoder_InitTypeDef enc = {0};
    enc.EncoderMode = (mode == ENC_MODE_TI1) ? TIM_ENCODERMODE_TI1 :   /* 计数模式 */
                      (mode == ENC_MODE_TI2) ? TIM_ENCODERMODE_TI2 :
                                               TIM_ENCODERMODE_TI12;
    enc.IC1Polarity = TIM_ICPOLARITY_RISING;  /* A 相上升沿有效 */
    enc.IC1Filter   = 0;                      /* 不滤波 */
    enc.IC2Polarity = TIM_ICPOLARITY_RISING;  /* B 相上升沿有效 */
    enc.IC2Filter   = 0;                      /* 不滤波 */
    HAL_TIM_Encoder_Init(htim, &enc);

    __HAL_TIM_SET_COUNTER(htim, 0);           /* 计数器清零 */

    ctx->htim      = htim;
    ctx->last_raw  = 0;
    ctx->position  = 0;
    ctx->wrap_mask = is_32bit ? 0xFFFFFFFF : 0xFFFF;
    ctx->init_flag = 1;                       /* 标记初始化完成 */
}

/**
 * @brief  启动编码器计数
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void enc_start (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    enc_ctx_t *ctx = &enc_ctx[idx];
    if (!ctx->init_flag) return;              /* 未初始化则返回 */
    HAL_TIM_Encoder_Start(ctx->htim, TIM_CHANNEL_ALL);
}

/**
 * @brief  停止编码器计数
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void enc_stop (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    enc_ctx_t *ctx = &enc_ctx[idx];
    if (!ctx->init_flag) return;              /* 未初始化则返回 */
    HAL_TIM_Encoder_Stop(ctx->htim, TIM_CHANNEL_ALL);
}

/**
 * @brief  获取编码器累加有符号位置
 *         读取硬件计数并与上次做有符号差值(内部处理计数器回绕),
 *         累加为 32 位有符号位置,正反方向自动带出
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 累加位置;未初始化或越界时返回 0
 * @note   相邻两次调用间计数增量不得超过计数范围的一半,否则无法区分方向
 */
int32 enc_get_count (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return 0;
    enc_ctx_t *ctx = &enc_ctx[idx];
    if (!ctx->init_flag) return 0;            /* 未初始化则返回 0 */

    uint32 raw   = __HAL_TIM_GET_COUNTER(ctx->htim);
    uint32 diff  = (raw - ctx->last_raw) & ctx->wrap_mask;  /* 无符号回绕差值 */

    int32 sdelta = (int32)diff;
    if (diff > (ctx->wrap_mask >> 1))         /* 超过半量程视为反向 */
        sdelta = (int32)diff - (int32)ctx->wrap_mask - 1;

    ctx->last_raw = raw;                      /* 更新上次计数值 */
    ctx->position += sdelta;                  /* 累加位置 */
    return ctx->position;
}

/**
 * @brief  获取编码器硬件原始计数值
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 当前计数值;未初始化或越界时返回 0
 * @note   16 位定时器在 0xFFFF 处回绕,如需连续位置请使用 enc_get_count
 */
uint32 enc_get_raw (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return 0;
    enc_ctx_t *ctx = &enc_ctx[idx];
    if (!ctx->init_flag) return 0;            /* 未初始化则返回 0 */
    return __HAL_TIM_GET_COUNTER(ctx->htim);
}

/**
 * @brief  编码器位置清零
 *         同时清零硬件计数器与软件累加位置
 * @param  idx: 定时器索引,见 tim_index_enum
 * @retval 无
 */
void enc_clear (tim_index_enum idx)
{
    if (idx >= TIM_COUNT) return;
    enc_ctx_t *ctx = &enc_ctx[idx];
    if (!ctx->init_flag) return;              /* 未初始化则返回 */

    __HAL_TIM_SET_COUNTER(ctx->htim, 0);      /* 清零硬件计数器 */
    ctx->last_raw = 0;
    ctx->position = 0;
}
