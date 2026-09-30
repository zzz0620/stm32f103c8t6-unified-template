/**
 * @file    DX_driver_spi.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   SPI 驱动头文件
 *          基于 STM32 SPI 外设封装主机模式下的初始化与收发接口
 */

#ifndef __DX_DRIVER_SPI_H
#define __DX_DRIVER_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief SPI 外设索引枚举
 *        用于选择 SPI1 或 SPI2
 */
typedef enum
{
    SPI_1 = 0,      /* SPI1 */
    SPI_2 = 1,      /* SPI2 */
    SPI_COUNT = 2   /* SPI 外设总数 */
} spi_index_enum;

/**
 * @brief SPI 工作模式枚举
 *        由 CPOL(时钟极性)与 CPHA(时钟相位)组合而成
 */
typedef enum
{
    SPI_MODE_0,     /* CPOL=0, CPHA=0 */
    SPI_MODE_1,     /* CPOL=0, CPHA=1 */
    SPI_MODE_2,     /* CPOL=1, CPHA=0 */
    SPI_MODE_3,     /* CPOL=1, CPHA=1 */
} spi_mode_enum;

/**
 * @brief SPI 引脚枚举
 *        列举支持的 SCK/MISO/MOSI 引脚组合及重映射配置
 */
typedef enum
{
    SPI1_SCK_PA5_MISO_PA6_MOSI_PA7,     /* SPI1: SCK=PA5, MISO=PA6, MOSI=PA7 */
    SPI1_SCK_PB3_MISO_PB4_MOSI_PB5,     /* SPI1: SCK=PB3, MISO=PB4, MOSI=PB5(重映射) */
    SPI2_SCK_PB13_MISO_PB14_MOSI_PB15,  /* SPI2: SCK=PB13, MISO=PB14, MOSI=PB15 */
} spi_pin_enum;

/* Exported functions prototypes ---------------------------------------------*/

void    spi_init             (spi_index_enum idx, spi_mode_enum mode, uint32 prescaler, spi_pin_enum pin);  /* SPI 初始化     */
uint8   spi_transfer         (spi_index_enum idx, uint8 data);                                            /* 单字节收发     */
void    spi_transfer_buffer  (spi_index_enum idx, const uint8 *tx, uint8 *rx, uint16 len);                /* 缓冲区批量收发 */

#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_SPI_H */
