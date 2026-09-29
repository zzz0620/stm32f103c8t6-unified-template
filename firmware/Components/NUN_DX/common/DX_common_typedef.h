#ifndef __DX_COMMON_TYPEDEF_H
#define __DX_COMMON_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "DX_config.h"              /* 工程配置开关(设备启用/禁用) */

/* Exported types ------------------------------------------------------------*/

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;

#ifdef __cplusplus
}
#endif

#endif /* __DX_COMMON_TYPEDEF_H */
