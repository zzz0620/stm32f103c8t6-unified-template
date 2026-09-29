/**
 * @file    DX_driver_dma.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   DMA 驱动头文件
 *         提供 STM32F1 DMA1 的初始化、启动、停止及回调管理的统一接口
 */

#ifndef __DX_DRIVER_DMA_H
#define __DX_DRIVER_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief DMA 通道枚举
 *        对应 DMA1 的 7 个通道
 */
typedef enum
{
    DMA_CH1,        /* DMA1 通道 1   */
    DMA_CH2,        /* DMA1 通道 2   */
    DMA_CH3,        /* DMA1 通道 3   */
    DMA_CH4,        /* DMA1 通道 4   */
    DMA_CH5,        /* DMA1 通道 5   */
    DMA_CH6,        /* DMA1 通道 6   */
    DMA_CH7,        /* DMA1 通道 7   */
    DMA_CH_COUNT    /* DMA 通道总数  */
} dma_channel_enum;

/**
 * @brief DMA 传输方向枚举
 */
typedef enum
{
    DMA_DIR_M2P,        // Memory → Peripheral (TX)
    DMA_DIR_P2M,        // Peripheral → Memory (RX)
} dma_dir_enum;

/**
 * @brief DMA 数据位宽枚举
 */
typedef enum
{
    DMA_WIDTH_8BIT,     /* 8 位   */
    DMA_WIDTH_16BIT,    /* 16 位  */
    DMA_WIDTH_32BIT,    /* 32 位  */
} dma_width_enum;

/**
 * @brief DMA 传输完成回调函数类型
 * @param  arg: 用户自定义上下文参数
 */
typedef void (*dma_callback_t)(void *arg);

/* Exported functions prototypes ---------------------------------------------*/

void    dma_init            (dma_channel_enum ch, dma_dir_enum dir, dma_width_enum width,
                             volatile void *periph_addr, void *mem_addr, uint32 size, uint8 circular);  /* 初始化 DMA 通道            */
void    dma_start           (dma_channel_enum ch, uint32 size);                                         /* 启动 DMA 传输              */
void    dma_stop            (dma_channel_enum ch);                                                      /* 停止 DMA 传输              */
uint32  dma_remaining       (dma_channel_enum ch);                                                      /* 查询剩余传输数据量         */
void    dma_set_callback    (dma_channel_enum ch, dma_callback_t cb, void *arg);                       /* 设置传输完成回调           */

DMA_HandleTypeDef * dma_get_handle (dma_channel_enum ch);                                              /* 获取 DMA 句柄              */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_DMA_H */
