/**
 * @file    DX_debug.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   调试模块头文件
 *          基于 UART 实现调试输出、断言、日志及 DMA 发送接口
 */

#ifndef __DX_DEBUG_H
#define __DX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"
#include "DX_driver_uart.h"

/* Exported defines ----------------------------------------------------------*/

#define DEBUG_UART_INDEX            ( UART_1 )              /* 调试使用的串口号                 */
#define DEBUG_UART_BAUDRATE         ( 115200 )              /* 调试串口波特率                   */
#define DEBUG_UART_PIN              ( UART1_TX_PA9_RX_PA10 )/* 调试串口引脚配置                 */

#define DEBUG_UART_USE_INTERRUPT    ( 0 )                   /* 是否启用串口中断接收            */

#if DEBUG_UART_USE_INTERRUPT
#define DEBUG_UART_IRQ_PRIORITY     ( 1 )                   /* 串口中断优先级(中断模式下生效) */
#endif

#define DEBUG_RING_BUFFER_LEN       ( 64 )                  /* 环形缓冲区长度                  */

/* Exported macro ------------------------------------------------------------*/

#define DX_assert(x)                ( debug_assert_handler((x), __FILE__, __LINE__) )               /* 断言宏:失败时进入断言处理   */
#define DX_log(x, str)              ( debug_log_handler((x), (str), __FILE__, __LINE__) )           /* 日志宏:失败时输出日志信息   */

/* Exported functions prototypes ---------------------------------------------*/

void        debug_init                  (void);              /* 调试模块初始化                  */
void        debug_test                   (void);              /* 调试测试函数                    */
void        debug_assert_test            (uint8 trigger_fail);/* 断言功能测试函数                */
void        debug_log_test               (void);              /* 日志功能测试函数                */
void        debug_assert_enable         (void);              /* 使能断言检查                    */
void        debug_assert_disable        (void);              /* 禁用断言检查                    */
void        debug_assert_handler        (uint8 pass, char *file, int line);   /* 断言处理函数    */
void        debug_log_handler           (uint8 pass, char *str, char *file, int line); /* 日志处理函数 */

uint32      debug_send_buffer           (const uint8 *buff, uint32 len);  /* 阻塞发送一段缓冲区   */
void        debug_send_dma              (const uint8 *buff, uint32 len);  /* DMA 发送一段缓冲区    */
void        debug_printf_dma            (const char *fmt, ...);           /* DMA 格式化打印        */
uint8       debug_dma_busy              (void);                           /* 查询 DMA 发送是否忙    */

#define     debug_dma_str(str)          debug_send_dma((const uint8 *)(str), strlen(str))  /* DMA 发送字符串宏 */
uint32      debug_read_ring_buffer      (uint8 *buff, uint32 len);  /* 从环形缓冲区读取数据(中断模式) */
void        debug_interrupt_handler     (void);                    /* 调试串口中断处理(中断模式)     */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DEBUG_H */
