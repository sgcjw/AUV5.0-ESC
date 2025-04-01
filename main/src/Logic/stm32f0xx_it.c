/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f0xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "Logic/stm32f0xx_it.h"
#include "Logic/main.h"

extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart2;
extern msg_queue uart_txbuf, uart_rxbuf;
extern msg_queue can_rxbuf0, can_rxbuf1;

extern volatile uint8_t isUartIdle;
extern volatile bool dropped_byte;
extern volatile bool dropped_byte_index;

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */

	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */

	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
	/* USER CODE BEGIN HardFault_IRQn 0 */

	/* USER CODE END HardFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
	/* USER CODE BEGIN SVC_IRQn 0 */

	/* USER CODE END SVC_IRQn 0 */
	/* USER CODE BEGIN SVC_IRQn 1 */

	/* USER CODE END SVC_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
	/* USER CODE BEGIN PendSV_IRQn 0 */

	/* USER CODE END PendSV_IRQn 0 */
	/* USER CODE BEGIN PendSV_IRQn 1 */

	/* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
	/* USER CODE BEGIN SysTick_IRQn 0 */

	/* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */

	/* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles HDMI-CEC and CAN global interrupts / HDMI-CEC wake-up interrupt through EXTI line 27.
 */
void CEC_CAN_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan);
}

void TIM17_IRQHandler(void) { }

void USART2_IRQHandler(void)
{
	// Ref.: STM RM0091 27.8 -- USART Interrupts

	// Some ISR flags are cleared automatically on specific sequences of read/writes
	uint32_t status = huart2.Instance->ISR;

	// TXE -- Transmit Data Register Empty
	if (status & USART_ISR_TXE)
		// No need to manually clear TC bit; will be automatically cleared after
		// a read from USART_SR and a write to USART_DR
		if (MsgQueue_Empty(&uart_txbuf) && (huart2.Instance->ISR & USART_ISR_TC)) {
			// Nothing else to send, TXEIE will be re-enabled by UART_TranStart
			huart2.Instance->CR1 &= ~USART_CR1_TXEIE;
		}
		else if (!MsgQueue_Empty(&uart_txbuf)) {
			huart2.Instance->TDR = MsgQueue_pop(&uart_txbuf);
		}

	// RXNE -- Received Data Ready to be Read
	// ORE -- Overrun Error Detected
	if (status & USART_ISR_RXNE || status & USART_ISR_ORE) {
		uint8_t d = huart2.Instance->RDR; // RDR read clears USART_ISR_RXNE

		// Byte is malformed; drop rather than transmit something potentially disastrous
		if (status & USART_ISR_NE || status & USART_ISR_FE || status & USART_ISR_PE || status & USART_ISR_ORE) {
			// Which specific flag is set is unimportant; clear all of them
			huart2.Instance->ICR |= (USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF);
			dropped_byte = true;
			dropped_byte_index = MsgQueue_getPushIndex(&uart_rxbuf);
		}
		else {
			MsgQueue_push(&uart_rxbuf, d);
		}
	}
}

//
//	CAN IRQ Handlers
//

/**
 * @brief Callback for CAN Receive Buffer 0. For IDs 0 to 15
 * @note Called from interrupt context
 *
 * @param hcan CAN handle
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
	CAN_RxHeaderTypeDef rxheader; // place where the received header will be stored
	uint8_t rxdata[12];

	HAL_CAN_GetRxMessage(hcan, 0, &rxheader, rxdata);

	uint8_t id = rxheader.StdId;
	uint8_t filterID = rxheader.FilterMatchIndex;
	uint8_t len = rxheader.DLC;
	uint8_t ret = 1;

	ret &= MsgQueue_push(&can_rxbuf0, id);
	ret &= MsgQueue_push(&can_rxbuf0, filterID);
	ret &= MsgQueue_push(&can_rxbuf0, len);
	for (uint8_t i = 0; i < len; i++) {
		ret &= MsgQueue_push(&can_rxbuf0, rxdata[i]);
	}

	if (ret == 0) {
		// for now, catch buffer full
		// while(1);
	}
}

/**
 * @brief Callback for CAN Receive Buffer 1. For IDs 16 and up
 * @note Called from interrupt context
 *
 * @param hcan CAN handle
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
	CAN_RxHeaderTypeDef rxheader; // place where the received header will be stored
	uint8_t rxdata[12];

	HAL_CAN_GetRxMessage(hcan, 1, &rxheader, rxdata);

	uint8_t id = rxheader.StdId;
	uint8_t len = rxheader.DLC;
	uint8_t filterID = rxheader.FilterMatchIndex;
	uint8_t ret = 1;

	ret &= MsgQueue_push(&can_rxbuf1, id);
	ret &= MsgQueue_push(&can_rxbuf1, filterID);
	ret &= MsgQueue_push(&can_rxbuf1, len);
	for (uint8_t i = 0; i < len; i++) {
		ret &= MsgQueue_push(&can_rxbuf1, rxdata[i]);
	}

	if (ret == 0) {
		// for now, catch buffer full
		// while(1);
	}
}
