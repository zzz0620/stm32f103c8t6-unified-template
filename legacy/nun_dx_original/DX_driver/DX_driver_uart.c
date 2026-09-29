/**
 * @file    DX_driver_uart.c
 * @author  YCZ
 * @date    2026-08-01
 * @brief   UART 串口驱动源文件
 *          基于 HAL 库封装 USART 初始化、收发、中断及 DMA 发送的实现
 */

#include "DX_driver_uart.h"
#include "DX_common_fifo.h"
#include "DX_common_interrupt.h"
#include "DX_driver_dma.h"

#define UART_FIFO_SIZE      (128)   /* UART 接收 FIFO 缓冲区大小 (字节) */

typedef struct
{
    GPIO_TypeDef *port;     /* 引脚所在 GPIO 端口 */
    uint16 tx_pin;          /* 发送引脚           */
    uint16 rx_pin;          /* 接收引脚           */
    uint32 remap_val;       /* 重映射配置值       */
    uint32 remap_mask;      /* 重映射配置掩码     */
} uart_pin_config_t;

/* 各 UART 引脚组合配置表,索引与 uart_pin_enum 一一对应 */
static const uart_pin_config_t pin_table[] =
{
    [UART1_TX_PA9_RX_PA10]  = { GPIOA, GPIO_PIN_9,  GPIO_PIN_10, 0x00000000, AFIO_MAPR_USART1_REMAP_Msk },
    [UART1_TX_PB6_RX_PB7]   = { GPIOB, GPIO_PIN_6,  GPIO_PIN_7,  AFIO_MAPR_USART1_REMAP_Msk, AFIO_MAPR_USART1_REMAP_Msk },
    [UART2_TX_PA2_RX_PA3]   = { GPIOA, GPIO_PIN_2,  GPIO_PIN_3,  0x00000000, AFIO_MAPR_USART2_REMAP_Msk },
    [UART2_TX_PD5_RX_PD6]   = { GPIOD, GPIO_PIN_5,  GPIO_PIN_6,  AFIO_MAPR_USART2_REMAP_Msk, AFIO_MAPR_USART2_REMAP_Msk },
    [UART3_TX_PB10_RX_PB11] = { GPIOB, GPIO_PIN_10, GPIO_PIN_11, AFIO_MAPR_USART3_REMAP_NOREMAP,  AFIO_MAPR_USART3_REMAP_Msk },
//    [UART3_TX_PC10_RX_PC11] = { GPIOC, GPIO_PIN_10, GPIO_PIN_11, AFIO_MAPR_USART3_REMAP_PARTIALREMAP, AFIO_MAPR_USART3_REMAP_Msk },
//    [UART3_TX_PD8_RX_PD9]   = { GPIOD, GPIO_PIN_8,  GPIO_PIN_9,  AFIO_MAPR_USART3_REMAP_FULLREMAP, AFIO_MAPR_USART3_REMAP_Msk },
};

typedef struct
{
    UART_HandleTypeDef         handle;     /* HAL UART 句柄              */
    uint8                      init_flag;  /* 初始化完成标志             */

    uint8                      fifo_buf[UART_FIFO_SIZE];  /* 接收 FIFO 缓冲区 */
    fifo_struct                fifo;       /* 接收 FIFO 管理结构         */

    void                      (*rx_idle_cb)(void *);   /* 接收空闲回调    */
    void                      (*rx_byte_cb)(void *);   /* 单字节接收回调  */
    void                      (*error_cb)(void *);     /* 错误回调        */
    void                      *cb_arg;                 /* 回调参数        */
} uart_ctx_t;

static uart_ctx_t uart_ctx[UART_COUNT];    /* 各 UART 上下文数组 */

static USART_TypeDef *const uart_instance[] =
{
    USART1, USART2, USART3
};

/**
 * @brief  使能指定 GPIO 端口时钟
 * @param  port: GPIO 端口 (如 GPIOA、GPIOB 等)
 * @retval 无
 */
static void uart_gpio_clk_enable (GPIO_TypeDef *port)
{
    if (port == GPIOA)       __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD)  __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE)  __HAL_RCC_GPIOE_CLK_ENABLE();
}

/**
 * @brief  使能指定 UART 外设时钟
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @retval 无
 */
static void uart_clk_enable (uart_index_enum idx)
{
    switch (idx)
    {
        case UART_1: __HAL_RCC_USART1_CLK_ENABLE(); break;
        case UART_2: __HAL_RCC_USART2_CLK_ENABLE(); break;
        case UART_3: __HAL_RCC_USART3_CLK_ENABLE(); break;
        default: break;
    }
}

/**
 * @brief  初始化 UART
 *         配置引脚复用、重映射、GPIO 及 UART 参数,并初始化接收 FIFO
 * @param  idx : UART 端口索引,见 uart_index_enum
 * @param  baud: 波特率 (如 115200)
 * @param  pin : 引脚映射组合,见 uart_pin_enum
 * @retval 无
 */
void uart_init (uart_index_enum idx, uint32 baud, uart_pin_enum pin)
{
    if (idx >= UART_COUNT) return;

    uart_ctx_t *ctx = &uart_ctx[idx];
    ctx->init_flag = 0;

    const uart_pin_config_t *pincfg = &pin_table[pin];

    __HAL_RCC_AFIO_CLK_ENABLE();

    uart_gpio_clk_enable(pincfg->port);
    uart_clk_enable(idx);

    MODIFY_REG(AFIO->MAPR, pincfg->remap_mask, pincfg->remap_val);  /* 配置引脚重映射 */

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = pincfg->tx_pin;
    gpio.Mode  = GPIO_MODE_AF_PP;    /* TX 引脚:复用推挽输出 */
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pincfg->port, &gpio);

    gpio.Pin   = pincfg->rx_pin;
    gpio.Mode  = GPIO_MODE_INPUT;    /* RX 引脚:上拉输入 */
    gpio.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(pincfg->port, &gpio);

    UART_HandleTypeDef *huart = &ctx->handle;
    huart->Instance          = uart_instance[idx];
    huart->Init.BaudRate     = baud;
    huart->Init.WordLength   = UART_WORDLENGTH_8B;      /* 8 位数据位   */
    huart->Init.StopBits     = UART_STOPBITS_1;         /* 1 位停止位   */
    huart->Init.Parity       = UART_PARITY_NONE;        /* 无校验       */
    huart->Init.Mode         = UART_MODE_TX_RX;         /* 收发模式     */
    huart->Init.HwFlowCtl    = UART_HWCONTROL_NONE;     /* 无硬件流控   */
    huart->Init.OverSampling = UART_OVERSAMPLING_16;    /* 16 倍过采样  */
    HAL_UART_Init(huart);

    fifo_init(&ctx->fifo, FIFO_DATA_8BIT, ctx->fifo_buf, UART_FIFO_SIZE);   /* 初始化接收 FIFO */

    ctx->init_flag = 1;
}

/**
 * @brief  发送单字节(阻塞)
 * @param  idx : UART 端口索引,见 uart_index_enum
 * @param  data: 待发送字节
 * @retval 无
 */
void uart_write_byte (uart_index_enum idx, uint8 data)
{
    if (idx >= UART_COUNT) return;
    HAL_UART_Transmit(&uart_ctx[idx].handle, &data, 1, 100);
}

/**
 * @brief  发送数据缓冲区(阻塞)
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @param  buf: 待发送数据缓冲区指针
 * @param  len: 待发送数据长度 (字节)
 * @retval 无
 */
void uart_write_buffer (uart_index_enum idx, const uint8 *buf, uint32 len)
{
    if (idx >= UART_COUNT) return;
    HAL_UART_Transmit(&uart_ctx[idx].handle, (uint8 *)buf, len, 1000);
}

/**
 * @brief  发送以 '\0' 结尾的字符串
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @param  str: 待发送字符串指针
 * @retval 无
 */
void uart_write_string (uart_index_enum idx, const char *str)
{
    if (idx >= UART_COUNT) return;
    while (*str)
    {
        uart_write_byte(idx, (uint8)*str++);
    }
}

/**
 * @brief  阻塞接收单字节
 * @param  idx : UART 端口索引,见 uart_index_enum
 * @param  data: 接收数据存放地址
 * @retval UART_STATUS_OK: 接收成功; UART_STATUS_TIMEOUT: 接收超时
 */
uart_status_enum uart_read_byte (uart_index_enum idx, uint8 *data)
{
    if (idx >= UART_COUNT) return UART_STATUS_TIMEOUT;
    if (HAL_UART_Receive(&uart_ctx[idx].handle, data, 1, 10) == HAL_OK)
        return UART_STATUS_OK;
    return UART_STATUS_TIMEOUT;
}

/**
 * @brief  从接收 FIFO 查询一字节(非阻塞)
 * @param  idx : UART 端口索引,见 uart_index_enum
 * @param  data: 接收数据存放地址
 * @retval 1: 读取成功; 0: FIFO 为空
 */
uint32 uart_query_byte (uart_index_enum idx, uint8 *data)
{
    if (idx >= UART_COUNT) return 0;
    fifo_state_enum ret = fifo_read_element(&uart_ctx[idx].fifo, data, FIFO_READ_AND_CLEAN);
    return (ret == FIFO_SUCCESS) ? 1 : 0;
}

/**
 * @brief  配置 UART 中断
 *         按需使能 RXNE/IDLE 中断,并在开启中断时使能对应 NVIC 通道
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @param  cfg: 中断配置,见 uart_interrupt_config_enum
 * @retval 无
 */
void uart_set_interrupt_config (uart_index_enum idx, uart_interrupt_config_enum cfg)
{
    if (idx >= UART_COUNT) return;

    uart_ctx_t *ctx = &uart_ctx[idx];
    UART_HandleTypeDef *huart = &ctx->handle;

    __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);   /* 先关闭 RXNE 中断 */
    __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);   /* 先关闭 IDLE 中断 */

    if (cfg & UART_INTERRUPT_RX)
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);    /* 使能接收非空中断 */
    if (cfg & UART_INTERRUPT_IDLE)
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);    /* 使能空闲线中断   */

    if (cfg != UART_INTERRUPT_NONE)
    {
        uint32 irq[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn };
        interrupt_enable((IRQn_Type)irq[idx]);    /* 使能对应 NVIC 通道 */
    }
}

/**
 * @brief  注册 UART 回调函数
 * @param  idx : UART 端口索引,见 uart_index_enum
 * @param  type: 回调类型,见 uart_callback_type_enum
 * @param  cb  : 回调函数指针
 * @param  arg : 回调参数
 * @retval 无
 */
void uart_set_callback (uart_index_enum idx, uart_callback_type_enum type, void (*cb)(void *), void *arg)
{
    if (idx >= UART_COUNT) return;

    uart_ctx_t *ctx = &uart_ctx[idx];
    ctx->cb_arg = arg;

    switch (type)
    {
        case UART_CALLBACK_RX_IDLE:  ctx->rx_idle_cb = cb;  break;
        case UART_CALLBACK_RX_BYTE:  ctx->rx_byte_cb = cb;  break;
        case UART_CALLBACK_ERROR:    ctx->error_cb   = cb;  break;
    }
}

/**
 * @brief  UART 中断服务处理
 *         处理 RXNE(单字节接收)与 IDLE(空闲线)中断,并触发对应回调
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @retval 无
 * @note   需在对应 USART 的 ISR 中调用本函数
 */
void uart_irq_handler (uart_index_enum idx)
{
    if (idx >= UART_COUNT) return;

    uart_ctx_t *ctx = &uart_ctx[idx];
    USART_TypeDef *usart = uart_instance[idx];
    uint32 sr = usart->SR;  /* 读取状态寄存器 */

    /* 接收非空中断:读取数据入 FIFO 并触发字节回调 */
    if ((sr & USART_SR_RXNE) && (usart->CR1 & USART_CR1_RXNEIE))
    {
        uint8 data = (uint8)(usart->DR & 0xFF);
        fifo_write_element(&ctx->fifo, data);
        if (ctx->rx_byte_cb)
            ctx->rx_byte_cb(ctx->cb_arg);
    }

    /* 空闲线中断:清除标志并触发空闲回调 */
    if ((sr & USART_SR_IDLE) && (usart->CR1 & USART_CR1_IDLEIE))
    {
        (void)usart->SR;   /* 读 SR */
        (void)usart->DR;   /* 再读 DR,以清除 IDLE 标志 */
        if (ctx->rx_idle_cb)
            ctx->rx_idle_cb(ctx->cb_arg);
    }
}

/* DMA TX ----------------------------------------------------------------*/

/* 各 UART 对应的 DMA 发送通道表 */
static const dma_channel_enum uart_dma_tx_ch[] =
{
    DMA_CH4, DMA_CH7, DMA_CH2     // USART1 TX, USART2 TX, USART3 TX
};

/**
 * @brief  初始化 UART DMA 发送
 *         配置 DMA 通道(内存到外设)并与 UART 句柄链接,使能 NVIC
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @retval 无
 */
void uart_dma_tx_init (uart_index_enum idx)
{
    if (idx >= UART_COUNT) return;

    uart_ctx_t *ctx = &uart_ctx[idx];
    USART_TypeDef  *usart = uart_instance[idx];
    dma_channel_enum ch = uart_dma_tx_ch[idx];

    dma_init(ch, DMA_DIR_M2P, DMA_WIDTH_8BIT,
             &usart->DR, NULL, 0, 0);   /* 内存 -> USART 数据寄存器 */

    __HAL_LINKDMA(&ctx->handle, hdmatx, *dma_get_handle(ch));  /* 将 DMA 句柄链接到 UART */

    IRQn_Type irq[] = { USART1_IRQn, USART2_IRQn, USART3_IRQn };
    interrupt_enable(irq[idx]);
}

/**
 * @brief  启动 UART DMA 发送
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @param  buf: 待发送数据缓冲区指针
 * @param  len: 待发送数据长度 (字节)
 * @retval 无
 */
void uart_dma_tx_start (uart_index_enum idx, const uint8 *buf, uint32 len)
{
    if (idx >= UART_COUNT) return;
    HAL_UART_Transmit_DMA(&uart_ctx[idx].handle, (uint8 *)buf, len);
}

/**
 * @brief  查询 DMA 发送是否忙碌
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @retval 1: 正在发送; 0: 空闲
 */
uint8 uart_dma_tx_busy (uart_index_enum idx)
{
    if (idx >= UART_COUNT) return 0;
    return (uart_ctx[idx].handle.gState == HAL_UART_STATE_BUSY_TX) ? 1 : 0;
}

/**
 * @brief  设置 DMA 发送完成回调
 * @param  idx: UART 端口索引,见 uart_index_enum
 * @param  cb : 回调函数指针
 * @param  arg: 回调参数
 * @retval 无
 */
void uart_dma_tx_set_callback (uart_index_enum idx, void (*cb)(void *), void *arg)
{
    if (idx >= UART_COUNT) return;
    dma_channel_enum ch = uart_dma_tx_ch[idx];
    dma_set_callback(ch, (dma_callback_t)cb, arg);
}

void USART1_IRQHandler (void) { HAL_UART_IRQHandler(&uart_ctx[UART_1].handle); }   /* USART1 中断服务函数 */
void USART2_IRQHandler (void) { HAL_UART_IRQHandler(&uart_ctx[UART_2].handle); }   /* USART2 中断服务函数 */
void USART3_IRQHandler (void) { HAL_UART_IRQHandler(&uart_ctx[UART_3].handle); }   /* USART3 中断服务函数 */
