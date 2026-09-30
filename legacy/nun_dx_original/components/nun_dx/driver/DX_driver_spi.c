/**
 * @file    DX_driver_spi.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   SPI 驱动源文件
 *          基于 HAL 库封装 SPI 主机模式下的初始化与单字节/缓冲区收发实现
 */

#include "DX_driver_spi.h"

/**
 * @brief SPI 引脚配置结构体
 *        描述某组 SPI 引脚对应的端口、SCK/MISO/MOSI 引脚号及重映射参数
 */
typedef struct
{
    GPIO_TypeDef *port;      /* GPIO 端口        */
    uint16 sck_pin;          /* SCK 引脚号       */
    uint16 miso_pin;         /* MISO 引脚号      */
    uint16 mosi_pin;         /* MOSI 引脚号      */
    uint32 remap_val;        /* 重映射配置值     */
    uint32 remap_mask;       /* 重映射掩码       */
} spi_pin_config_t;

/* SPI 引脚映射表:枚举值 -> 端口/SCK/MISO/MOSI/重映射参数 */
static const spi_pin_config_t spi_pin_table[] =
{
    [SPI1_SCK_PA5_MISO_PA6_MOSI_PA7]    = { GPIOA, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, 0x00000000, AFIO_MAPR_SPI1_REMAP_Msk },
    [SPI1_SCK_PB3_MISO_PB4_MOSI_PB5]    = { GPIOB, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, AFIO_MAPR_SPI1_REMAP_Msk, AFIO_MAPR_SPI1_REMAP_Msk },
    [SPI2_SCK_PB13_MISO_PB14_MOSI_PB15] = { GPIOB, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, 0x00000000, 0 },
};

/**
 * @brief SPI 运行上下文结构体
 *        保存 SPI 句柄与初始化标志
 */
typedef struct
{
    SPI_HandleTypeDef handle;    /* SPI 句柄          */
    uint8             init_flag; /* 初始化完成标志    */
} spi_ctx_t;

static spi_ctx_t spi_ctx[SPI_COUNT];  /* 各 SPI 外设的运行上下文 */

/* SPI 外设实例指针表,索引与 spi_index_enum 对应 */
static SPI_TypeDef *const spi_instance[] =
{
    SPI1, SPI2
};

/**
 * @brief  使能 GPIO 端口时钟
 * @param  port: GPIO 端口 (如 GPIOA、GPIOB)
 * @retval 无
 */
static void spi_gpio_clk_enable (GPIO_TypeDef *port)
{
    if (port == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
 * @brief  SPI 初始化
 *         配置 GPIO 复用与引脚重映射,按指定模式与分频系数初始化 SPI 主机,
 *         并将句柄保存至 spi_ctx 供后续收发使用
 * @param  idx      : SPI 外设索引,见 spi_index_enum
 * @param  mode     : SPI 工作模式,见 spi_mode_enum
 * @param  prescaler: 波特率分频系数
 * @param  pin      : SPI 引脚配置,见 spi_pin_enum
 * @retval 无
 */
void spi_init (spi_index_enum idx, spi_mode_enum mode, uint32 prescaler, spi_pin_enum pin)
{
    if (idx >= SPI_COUNT) return;  /* 参数合法性校验 */

    spi_ctx_t *ctx = &spi_ctx[idx];
    ctx->init_flag = 0;            /* 初始化期间清除标志 */

    const spi_pin_config_t *pincfg = &spi_pin_table[pin];

    __HAL_RCC_AFIO_CLK_ENABLE();          /* 使能 AFIO 时钟(重映射所需) */
    spi_gpio_clk_enable(pincfg->port);    /* 使能对应 GPIO 端口时钟 */

    MODIFY_REG(AFIO->MAPR, pincfg->remap_mask, pincfg->remap_val);  /* 配置引脚重映射 */

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = pincfg->sck_pin | pincfg->miso_pin | pincfg->mosi_pin;
    gpio.Mode  = GPIO_MODE_AF_PP;         /* 复用推挽输出 */
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;    /* 高速 */
    HAL_GPIO_Init(pincfg->port, &gpio);

    if (idx == SPI_1) __HAL_RCC_SPI1_CLK_ENABLE();  /* 使能对应 SPI 外设时钟 */
    if (idx == SPI_2) __HAL_RCC_SPI2_CLK_ENABLE();

    SPI_HandleTypeDef *hspi = &ctx->handle;
    hspi->Instance               = spi_instance[idx];
    hspi->Init.Mode              = SPI_MODE_MASTER;             /* 主机模式 */
    hspi->Init.Direction         = SPI_DIRECTION_2LINES;        /* 双线全双工 */
    hspi->Init.DataSize          = SPI_DATASIZE_8BIT;           /* 8 位数据帧 */
    hspi->Init.CLKPolarity       = (mode == SPI_MODE_2 || mode == SPI_MODE_3) ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW;  /* 时钟极性 */
    hspi->Init.CLKPhase          = (mode == SPI_MODE_1 || mode == SPI_MODE_3) ? SPI_PHASE_2EDGE : SPI_PHASE_1EDGE;     /* 时钟相位 */
    hspi->Init.NSS               = SPI_NSS_SOFT;                /* 软件管理 NSS */
    hspi->Init.BaudRatePrescaler = prescaler;                   /* 波特率分频 */
    hspi->Init.FirstBit          = SPI_FIRSTBIT_MSB;            /* 高位在前 */
    hspi->Init.TIMode            = SPI_TIMODE_DISABLE;          /* 关闭 TI 模式 */
    hspi->Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;  /* 关闭 CRC 计算 */
    hspi->Init.CRCPolynomial     = 7;
    HAL_SPI_Init(hspi);

    ctx->init_flag = 1;            /* 标记初始化完成 */
}

/**
 * @brief  SPI 单字节收发
 * @param  idx : SPI 外设索引,见 spi_index_enum
 * @param  data: 待发送的字节
 * @retval 接收到的字节
 */
uint8 spi_transfer (spi_index_enum idx, uint8 data)
{
    if (idx >= SPI_COUNT) return 0;          /* 参数合法性校验 */
    spi_ctx_t *ctx = &spi_ctx[idx];
    if (!ctx->init_flag) return 0;           /* 未初始化则返回 0 */

    uint8 rx;
    HAL_SPI_TransmitReceive(&ctx->handle, &data, &rx, 1, 100);  /* 同步收发 1 字节,超时 100ms */
    return rx;
}

/**
 * @brief  SPI 缓冲区批量收发
 * @param  idx: SPI 外设索引,见 spi_index_enum
 * @param  tx : 发送缓冲区指针
 * @param  rx : 接收缓冲区指针
 * @param  len: 收发字节数
 * @retval 无
 */
void spi_transfer_buffer (spi_index_enum idx, const uint8 *tx, uint8 *rx, uint16 len)
{
    if (idx >= SPI_COUNT) return;            /* 参数合法性校验 */
    spi_ctx_t *ctx = &spi_ctx[idx];
    if (!ctx->init_flag) return;             /* 未初始化则退出 */

    HAL_SPI_TransmitReceive(&ctx->handle, (uint8 *)tx, rx, len, 100);  /* 同步收发 len 字节,超时 100ms */
}
