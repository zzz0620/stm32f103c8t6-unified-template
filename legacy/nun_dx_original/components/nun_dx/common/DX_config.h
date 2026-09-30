/**
 * @file    DX_config.h
 * @author  YCZ
 * @date    2026-09-29
 * @brief   工程配置开关头文件
 *          集中管理各设备模块的启用/禁用开关:
 *          置 0 的模块 .c 整体被 #if 剔除,不编译、不占 Flash,
 *          统一头文件也不再包含其 API,避免误用
 */



#ifndef __DX_CONFIG_H
#define __DX_CONFIG_H

/*
 * Modified for the integrated template: feature ownership moved to one board-
 * level file so application code no longer edits third-party component files.
 */
#include "stm32_template_config.h"

/* DX_driver stays available; unused functions are discarded by the linker. */

#endif /* __DX_CONFIG_H */
