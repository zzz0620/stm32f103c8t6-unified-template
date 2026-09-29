/**
 * @file    DX_driver_adc.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   ADC 驱动源文件
 *         基于 HAL 库封装 ADC 初始化、通道配置与读取的实现
 */

#include "DX_driver_adc.h"

/**
 * @brief ADC 通道对应的 GPIO 引脚映射结构
 */
typedef struct
{
    GPIO_TypeDef *port; /* GPIO 端口 */
    uint16 pin;         /* 引脚号   */
} adc_channel_gpio_t;

/* ADC 通道与 GPIO 引脚的映射表 (CH0~CH15) */
static const adc_channel_gpio_t adc_gpio_table[] =
{
    { GPIOA, GPIO_PIN_0 },  // CH0
    { GPIOA, GPIO_PIN_1 },  // CH1
    { GPIOA, GPIO_PIN_2 },  // CH2
    { GPIOA, GPIO_PIN_3 },  // CH3
    { GPIOA, GPIO_PIN_4 },  // CH4
    { GPIOA, GPIO_PIN_5 },  // CH5
    { GPIOA, GPIO_PIN_6 },  // CH6
    { GPIOA, GPIO_PIN_7 },  // CH7
    { GPIOB, GPIO_PIN_0 },  // CH8
    { GPIOB, GPIO_PIN_1 },  // CH9
    { GPIOC, GPIO_PIN_0 },  // CH10
    { GPIOC, GPIO_PIN_1 },  // CH11
    { GPIOC, GPIO_PIN_2 },  // CH12
    { GPIOC, GPIO_PIN_3 },  // CH13
    { GPIOC, GPIO_PIN_4 },  // CH14
    { GPIOC, GPIO_PIN_5 },  // CH15
};

/**
 * @brief ADC 上下文结构
 *        保存句柄、初始化标志及各通道使能状态
 */
typedef struct
{
    ADC_HandleTypeDef   handle;        /* HAL ADC 句柄                   */
    uint8               init_flag;    /* 初始化完成标志 (1=已初始化)     */
    uint8               ch_active[16];/* 通道使能标志数组               */
} adc_ctx_t;

static adc_ctx_t adc_ctx[ADC_COUNT];  /* ADC1/ADC2 上下文实例 */

/* ADC 实例数组,索引与 adc_index_enum 对应 */
static ADC_TypeDef *const adc_instance[] =
{
    ADC1, ADC2
};

/**
 * @brief  初始化 ADC 通道对应的 GPIO 为模拟模式
 * @param  channel: ADC 通道号 (0~15)
 * @retval 无
 * @note   根据 adc_gpio_table 配置对应引脚为模拟输入,并使能 GPIO 时钟
 */
static void adc_gpio_init (uint8 channel)
{
    if (channel > 15) return;   /* 通道号越界直接返回 */

    const adc_channel_gpio_t *gpio = &adc_gpio_table[channel];

    /* 根据端口使能对应 GPIO 时钟 */
    if (gpio->port == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (gpio->port == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (gpio->port == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio_cfg = {0};
    gpio_cfg.Pin  = gpio->pin;
    gpio_cfg.Mode = GPIO_MODE_ANALOG;   /* 配置为模拟模式 */
    HAL_GPIO_Init(gpio->port, &gpio_cfg);
}

/**
 * @brief  初始化指定 ADC
 * @param  idx: ADC 索引,见 adc_index_enum (ADC_1 / ADC_2)
 * @retval 无
 */
void adc_init (adc_index_enum idx)
{
    if (idx >= ADC_COUNT) return;  /* 索引越界直接返回 */

    adc_ctx_t *ctx = &adc_ctx[idx];

    /* 使能对应 ADC 时钟 */
    if (idx == ADC_1) __HAL_RCC_ADC1_CLK_ENABLE();
    if (idx == ADC_2) __HAL_RCC_ADC2_CLK_ENABLE();

    ADC_HandleTypeDef *hadc = &ctx->handle;
    hadc->Instance                   = adc_instance[idx];
    hadc->Init.ScanConvMode          = ADC_SCAN_DISABLE;        /* 禁用扫描模式 */
    hadc->Init.ContinuousConvMode    = DISABLE;                  /* 禁用连续转换 */
    hadc->Init.DiscontinuousConvMode = DISABLE;                  /* 禁用间断模式 */
    hadc->Init.ExternalTrigConv      = ADC_SOFTWARE_START;       /* 软件触发     */
    hadc->Init.DataAlign             = ADC_DATAALIGN_RIGHT;      /* 数据右对齐   */
    hadc->Init.NbrOfConversion       = 1;                        /* 转换数量为 1 */
    HAL_ADC_Init(hadc);

    memset(ctx->ch_active, 0, sizeof(ctx->ch_active));   /* 清零通道使能标志 */
    ctx->init_flag = 1;                                   /* 标记初始化完成   */
}

/**
 * @brief  配置 ADC 通道
 * @param  idx         : ADC 索引,见 adc_index_enum
 * @param  channel     : ADC 通道号 (0~15)
 * @param  sample_time : 采样时间,见 adc_sampletime_enum
 * @retval 无
 */
void adc_channel_config (adc_index_enum idx, uint8 channel, adc_sampletime_enum sample_time)
{
    if (idx >= ADC_COUNT || channel > 15) return;   /* 参数越界直接返回 */

    adc_ctx_t *ctx = &adc_ctx[idx];
    if (!ctx->init_flag) return;                     /* ADC 未初始化直接返回 */

    adc_gpio_init(channel);                          /* 初始化通道对应 GPIO */

    ADC_ChannelConfTypeDef ch_cfg = {0};
    ch_cfg.Channel      = channel;                  /* 配置通道号      */
    ch_cfg.Rank         = ADC_REGULAR_RANK_1;        /* 规则组排名 1    */
    ch_cfg.SamplingTime = sample_time;               /* 设置采样时间    */
    HAL_ADC_ConfigChannel(&ctx->handle, &ch_cfg);

    ctx->ch_active[channel] = 1;                    /* 标记通道已使能 */
}

/**
 * @brief  读取 ADC 通道转换值
 * @param  idx     : ADC 索引,见 adc_index_enum
 * @param  channel : ADC 通道号 (0~15)
 * @retval 12 位转换结果 (0~4095);出错返回 0
 * @note   每次读取会重新配置通道并以软件触发一次单次转换,采样时间固定为 239.5 周期
 */
uint16 adc_read (adc_index_enum idx, uint8 channel)
{
    if (idx >= ADC_COUNT || channel > 15) return 0;   /* 参数越界返回 0 */

    adc_ctx_t *ctx = &adc_ctx[idx];
    if (!ctx->init_flag || !ctx->ch_active[channel]) return 0;  /* 未初始化或通道未使能返回 0 */

    ADC_ChannelConfTypeDef ch_cfg = {0};
    ch_cfg.Channel      = channel;
    ch_cfg.Rank         = ADC_REGULAR_RANK_1;
    ch_cfg.SamplingTime = ADC_SAMPLETIME_239C5;       /* 读取时使用最长采样时间 */
    HAL_ADC_ConfigChannel(&ctx->handle, &ch_cfg);

    HAL_ADC_Start(&ctx->handle);                      /* 启动 ADC 转换           */
    HAL_ADC_PollForConversion(&ctx->handle, 10);      /* 等待转换完成,超时 10ms  */
    uint16 val = HAL_ADC_GetValue(&ctx->handle);      /* 获取转换结果            */
    HAL_ADC_Stop(&ctx->handle);                       /* 停止 ADC                */

    return val;
}

/**
 * @brief  ADC 注入组转换完成回调 (弱定义)
 * @param  hadc: ADC 句柄指针
 * @retval 无
 * @note   默认空实现,用户可在外部重写以响应注入组转换完成事件
 */
__weak void HAL_ADCEx_InjectedConvCpltCallback (ADC_HandleTypeDef *hadc)
{
    (void)hadc;
}


/**
 * @brief  ADC 测试函数
 * @param  无
 * @retval 无
 * @note   无
 */

void adc_test (void){
	
	adc_init(ADC_1);
	
	adc_channel_config(ADC_1,0,ADC_SAMPLETIME_28C5);
	
	printf("ADC:%d   V:%.2f V\r\n",adc_read(ADC_1,0),adc_read(ADC_1,0)/4095*3.3);

	
}
