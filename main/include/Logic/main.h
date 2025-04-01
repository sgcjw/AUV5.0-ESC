/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include "can_lib.h"
#include "cobs.h"
#include "common/bb_can.h"
#include "fletcher.h"
#include "msg_queue.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_cortex.h"
#include "w5500_wrapper.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern msg_queue uart_txbuf, uart_rxbuf;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void ForwardToCAN_Routine();
void ForwardToCAN_NewRoutine();
void ForwardFromCAN_Routine();
void UDP_Routine();
void SendHeartbeat();
void PingUART();

// Utility functions
void UART_RecvStart();
void UART_RecvPause();
void UART_RecvResume();
void UART_TranStart();
void UART_WaitIdle(uint32_t timeout);

void CAN_SetAllFilters();
void CAN_RecvStart();
void CAN_RecvPause();
void CAN_RecvResume();
void CAN_TranStart();

bool isMsgForSBC(uint8_t can_fifo_num, uint8_t filter_num);
bool isMsgForOCS(uint8_t can_fifo_num, uint8_t filter_num);
/* USER CODE END EFP */

/* Private defines */

/* USER CODE BEGIN Private defines */
#define START_BYTE        254  // (uint8_t) 254 ==> (int8_t) -2
#define CAN_HB_INTERVAL   500  // 2 Hz
#define W5500_INTERVAL    100  // 10 Hz
#define DBG_PING_INTERVAL 1000 // 1Hz
/*
  // LEVEL 0

  // LEVEL 1

  // LEVEL 2

*/
#define DEBUG_LEVEL 1
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
