/**
 * @file    DX_driver_uart.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   UART 串口驱动头文件
 *          提供 STM32 USART 的初始化、收发、中断及 DMA 发送的统一接口
 */

#ifndef __DX_DRIVER_UART_H
#define __DX_DRIVER_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief UART 端口索引枚举
 *        对应 STM32F1 的 USART1/USART2/USART3
 */
typedef enum
{
    UART_1 = 0,     /* USART1 */
    UART_2 = 1,     /* USART2 */
    UART_3 = 2,     /* USART3 */
    UART_COUNT = 3
} uart_index_enum;

/**
 * @brief UART 引脚映射枚举
 *        列举各 USART 可选的 TX/RX 引脚组合(含重映射)
 */
typedef enum
{
    UART1_TX_PA9_RX_PA10,
    UART1_TX_PB6_RX_PB7,
    UART2_TX_PA2_RX_PA3,
    UART2_TX_PD5_RX_PD6,
    UART3_TX_PB10_RX_PB11,
//    UART3_TX_PC10_RX_PC11,
//    UART3_TX_PD8_RX_PD9,
} uart_pin_enum;

/**
 * @brief UART 中断使能配置枚举
 *        可按位组合以同时使能多种中断
 */
typedef enum
{
    UART_INTERRUPT_NONE    = 0x00,  /* 不使能中断          */
    UART_INTERRUPT_RX      = 0x01,  /* 接收非空中断 (RXNE) */
    UART_INTERRUPT_IDLE    = 0x02,  /* 空闲线中断 (IDLE)   */
} uart_interrupt_config_enum;

/**
 * @brief UART 回调类型枚举
 *        用于指定注册回调的种类
 */
typedef enum
{
    UART_CALLBACK_RX_IDLE,  /* 接收空闲回调        */
    UART_CALLBACK_RX_BYTE,  /* 单字节接收回调      */
    UART_CALLBACK_ERROR,    /* 错误回调            */
} uart_callback_type_enum;

/**
 * @brief UART 操作状态枚举
 */
typedef enum
{
    UART_STATUS_OK,         /* 操作成功            */
    UART_STATUS_TIMEOUT,    /* 操作超时            */
    UART_STATUS_BUSY,       /* 串口忙碌            */
} uart_status_enum;

/* Exported functions prototypes ---------------------------------------------*/

void            uart_init                   (uart_index_enum idx, uint32 baud, uart_pin_enum pin);   /* 初始化 UART               */

void            uart_write_byte             (uart_index_enum idx, uint8 data);                      /* 发送单字节                */
void            uart_write_buffer           (uart_index_enum idx, const uint8 *buf, uint32 len);    /* 发送数据缓冲区            */
void            uart_write_string           (uart_index_enum idx, const char *str);                 /* 发送字符串                */

uart_status_enum uart_read_byte             (uart_index_enum idx, uint8 *data);                     /* 阻塞接收单字节            */

uint32          uart_query_byte             (uart_index_enum idx, uint8 *data);                     /* 从 FIFO 查询接收字节      */

void            uart_set_interrupt_config   (uart_index_enum idx, uart_interrupt_config_enum cfg);  /* 配置 UART 中断            */
void            uart_set_callback           (uart_index_enum idx, uart_callback_type_enum type, void (*cb)(void *), void *arg);  /* 注册回调函数 */

void            uart_irq_handler            (uart_index_enum idx);                                  /* UART 中断服务处理         */

void            uart_dma_tx_init            (uart_index_enum idx);                                  /* 初始化 DMA 发送           */
void            uart_dma_tx_start           (uart_index_enum idx, const uint8 *buf, uint32 len);    /* 启动 DMA 发送             */
uint8           uart_dma_tx_busy            (uart_index_enum idx);                                  /* 查询 DMA 发送是否忙碌     */
void            uart_dma_tx_set_callback    (uart_index_enum idx, void (*cb)(void *), void *arg);   /* 设置 DMA 发送完成回调     */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_UART_H */
