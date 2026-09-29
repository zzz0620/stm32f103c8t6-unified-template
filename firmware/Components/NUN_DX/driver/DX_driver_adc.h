/**
 * @file    DX_driver_adc.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   ADC 驱动头文件
 *         提供 STM32F1 ADC 的初始化、通道配置与读取的统一接口
 */

#ifndef __DX_DRIVER_ADC_H
#define __DX_DRIVER_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_common_typedef.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ADC 索引枚举
 *        用于选择使用 ADC1 或 ADC2
 */
typedef enum
{
    ADC_1 = 0,          /* ADC1 索引        */
    ADC_2 = 1,          /* ADC2 索引        */
    ADC_COUNT = 2       /* ADC 总数         */
} adc_index_enum;

/**
 * @brief ADC 采样时间枚举
 *        对 HAL 库采样时间宏进行封装,便于统一调用
 */
typedef enum
{
    ADC_SAMPLETIME_1C5   = ADC_SAMPLETIME_1CYCLE_5,      /* 1.5 个采样周期    */
    ADC_SAMPLETIME_7C5   = ADC_SAMPLETIME_7CYCLES_5,     /* 7.5 个采样周期    */
    ADC_SAMPLETIME_13C5  = ADC_SAMPLETIME_13CYCLES_5,    /* 13.5 个采样周期   */
    ADC_SAMPLETIME_28C5  = ADC_SAMPLETIME_28CYCLES_5,    /* 28.5 个采样周期   */
    ADC_SAMPLETIME_41C5  = ADC_SAMPLETIME_41CYCLES_5,    /* 41.5 个采样周期   */
    ADC_SAMPLETIME_55C5  = ADC_SAMPLETIME_55CYCLES_5,    /* 55.5 个采样周期   */
    ADC_SAMPLETIME_71C5  = ADC_SAMPLETIME_71CYCLES_5,    /* 71.5 个采样周期   */
    ADC_SAMPLETIME_239C5 = ADC_SAMPLETIME_239CYCLES_5,   /* 239.5 个采样周期  */
} adc_sampletime_enum;

/* Exported functions prototypes ---------------------------------------------*/

void    adc_init                (adc_index_enum idx);                                                    /* 初始化指定 ADC                */
void    adc_channel_config      (adc_index_enum idx, uint8 channel, adc_sampletime_enum sample_time);    /* 配置 ADC 通道及采样时间       */
uint16  adc_read                (adc_index_enum idx, uint8 channel);                                     /* 读取 ADC 通道转换值           */
void 		adc_test 								(void);																																	/* 读取 ADC 通道测试函数           */
#ifdef __cplusplus
}
#endif

#endif /* __DX_DRIVER_ADC_H */
