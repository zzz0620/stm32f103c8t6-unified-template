/**
 * @file    DX_device_key.h
 * @author  YCZ
 * @date    2026-08-01
 * @brief   按键设备头文件
 *          提供多按键扫描、状态识别(短按/长按/释放)的统一接口
 */

#ifndef __DX_DEVICE_KEY_H
#define __DX_DEVICE_KEY_H
#include "DX_common_typedef.h"      /* 先引入配置开关(DX_config.h),保证下方 #if 可见 */
#if DX_USE_KEY

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DX_driver_gpio.h"
#include "DX_common_headfile.h"

/* Exported defines ----------------------------------------------------------*/

#define KEY_MAX_COUNT       8   /* 按键池最大数量(最多支持 8 个按键) */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 按键状态枚举
 *        用于描述按键当前产生的事件类型
 */
typedef enum
{
    KEY_NONE,       /* 无事件           */
    KEY_PRESSED,    /* 按下事件         */
    KEY_RELEASED,   /* 释放事件         */
    KEY_HOLD,       /* 长按事件         */
} key_state_enum;

/*
  用户扩展按键名：在 KEY_A 前追加即可
  typedef enum { KEY_UP, KEY_DOWN, KEY_OK, KEY_MAX = KEY_MAX_COUNT } key_name_enum;
*/
typedef enum
{
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,  /* 按键编号(对应按键池索引) */
} key_name_enum;

/* Exported functions prototypes ---------------------------------------------*/

void            key_bind    (key_name_enum name, GPIO_TypeDef *port, uint16 pin);  /* 绑定按键到指定 GPIO 引脚 */
void            key_scan    (void);                                               /* 扫描所有按键并更新状态    */
key_state_enum  key_get     (key_name_enum name);                                 /* 获取按键状态(读取后清零) */

void key_test (void);
	
#ifdef __cplusplus
}
#endif

#endif /* DX_USE_KEY */
#endif /* __DX_DEVICE_KEY_H */
