/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */
#include <string.h>

#include "dart_board_link.h"

static dart_link_command_t g_host_command;
static volatile uint8_t g_host_command_valid = 0u;
static volatile uint8_t g_loadcell_can_started = 0u;

volatile uint32_t g_loadcell_can_rx_irq_count = 0u;
volatile uint32_t g_loadcell_can_rx_msg_count = 0u;
volatile uint32_t g_loadcell_can_rx_poll_count = 0u;
volatile uint32_t g_loadcell_can_rx_last_id = 0u;
volatile uint32_t g_loadcell_can_rx_last_tick = 0u;
volatile uint8_t g_loadcell_can_rx_last_dlc = 0u;
volatile uint32_t g_loadcell_can_tx_ok_count = 0u;
volatile uint32_t g_loadcell_can_tx_err_count = 0u;
volatile uint32_t g_loadcell_can_tx_last_id = 0u;
volatile uint32_t g_loadcell_can_tx_last_hal_status = 0u;
volatile uint32_t g_loadcell_can_tx_last_error = 0u;
volatile uint32_t g_loadcell_can_tx_last_esr = 0u;
volatile uint32_t g_loadcell_can_tx_last_tsr = 0u;
volatile uint32_t g_loadcell_can_tx_last_free_level = 0u;
volatile uint32_t g_loadcell_can_start_count = 0u;
volatile uint32_t g_loadcell_can_start_fail_count = 0u;
volatile uint32_t g_loadcell_can_last_state = 0u;
volatile uint32_t g_loadcell_can_last_error = 0u;

static void loadcell_can_config_filter(void)
{
  CAN_FilterTypeDef can_filter_st = {0};

  can_filter_st.FilterActivation = ENABLE;
  can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
  can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
  can_filter_st.FilterIdHigh = (uint16_t)(DART_LINK_CAN_HOST_COMMAND_ID << 5);
  can_filter_st.FilterIdLow = 0x0000;
  can_filter_st.FilterMaskIdHigh = (uint16_t)(0x7FFU << 5);
  can_filter_st.FilterMaskIdLow = 0x0000;
  can_filter_st.FilterBank = 0;
  can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;

  (void)HAL_CAN_ConfigFilter(&hcan, &can_filter_st);
}

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 3;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_9TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* USER CODE BEGIN 1 */
void LoadcellCanStart(void)
{
  g_loadcell_can_start_count++;
  if (g_loadcell_can_started == 0u)
  {
    HAL_CAN_StateTypeDef state;

    loadcell_can_config_filter();
    state = HAL_CAN_GetState(&hcan);
    g_loadcell_can_last_state = (uint32_t)state;
    g_loadcell_can_last_error = HAL_CAN_GetError(&hcan);

    if ((state == HAL_CAN_STATE_READY) || (state == HAL_CAN_STATE_RESET))
    {
      if (HAL_CAN_Start(&hcan) != HAL_OK)
      {
        g_loadcell_can_start_fail_count++;
        g_loadcell_can_last_state = (uint32_t)HAL_CAN_GetState(&hcan);
        g_loadcell_can_last_error = HAL_CAN_GetError(&hcan);
        return;
      }
    }

    g_loadcell_can_started = 1u;
  }

  (void)HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

bool loadcell_can_get_host_command(dart_link_command_t *out)
{
  if ((out == NULL) || (g_host_command_valid == 0u))
  {
    return false;
  }

  *out = g_host_command;
  return true;
}

void loadcell_can_store_host_command(const dart_link_command_t *command)
{
  if (command == NULL)
  {
    return;
  }

  g_host_command = *command;
  g_host_command_valid = 1u;
}

/* USER CODE END 1 */
