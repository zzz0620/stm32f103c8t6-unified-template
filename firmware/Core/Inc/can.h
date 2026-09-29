/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "dart_board_link.h"

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN_Init(void);

void LoadcellCanStart(void);
bool loadcell_can_get_host_command(dart_link_command_t *out);
void loadcell_can_store_host_command(const dart_link_command_t *command);

extern volatile uint32_t g_loadcell_can_rx_irq_count;
extern volatile uint32_t g_loadcell_can_rx_msg_count;
extern volatile uint32_t g_loadcell_can_rx_poll_count;
extern volatile uint32_t g_loadcell_can_rx_last_id;
extern volatile uint32_t g_loadcell_can_rx_last_tick;
extern volatile uint8_t g_loadcell_can_rx_last_dlc;
extern volatile uint32_t g_loadcell_can_tx_ok_count;
extern volatile uint32_t g_loadcell_can_tx_err_count;
extern volatile uint32_t g_loadcell_can_tx_last_id;
extern volatile uint32_t g_loadcell_can_tx_last_hal_status;
extern volatile uint32_t g_loadcell_can_tx_last_error;
extern volatile uint32_t g_loadcell_can_tx_last_esr;
extern volatile uint32_t g_loadcell_can_tx_last_tsr;
extern volatile uint32_t g_loadcell_can_tx_last_free_level;
extern volatile uint32_t g_loadcell_can_start_count;
extern volatile uint32_t g_loadcell_can_start_fail_count;
extern volatile uint32_t g_loadcell_can_last_state;
extern volatile uint32_t g_loadcell_can_last_error;

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

