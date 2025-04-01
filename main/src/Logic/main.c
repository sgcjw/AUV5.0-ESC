/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 *
 * Logic Board for ASV4.0
 *
 * Written By: Justin2, Justin2, Yijian
 *
 *  Summary:
 *  - Read Message from SBC
 *  - Forward Message to PCBs
 *  - Read Message from PCBs
 *  - Forward Message to SBC
 *
 *  Not Done:
 *  - Set Filter and IDs properly
 *  - Send Data to UDP
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "Logic/main.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "sdcard.h"
#include "stdarg.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common/bb_can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

TIM_HandleTypeDef htim17;

/* USER CODE BEGIN PV */
msg_queue uart_txbuf; // Modified in USART2_IRQ (TXE)
msg_queue uart_rxbuf; // Modified in USART2_IRQ (RXNE / ORE)
msg_queue can_txbuf;
msg_queue can_rxbuf0;  // Modified in CEC_CAN_IRQ
msg_queue can_rxbuf1;  // Modified in CEC_CAN_IRQ
msg_queue udp_txbuf;   // Modified in CEC_CAN_IRQ
msg_queue current_msg; // temp buffer for reading from CAN

uint32_t can_hb_timer = 0;
uint32_t dbg_ping_timer = 0;

// UART frame processing state machine
volatile bool dropped_byte = false;
volatile uint8_t dropped_byte_index = 0;
uint8_t incoming_byte = 0;
uint8_t read_buffer[8];      // Buffer for packet data
uint32_t read_start_ctr = 0; // Number of START_BYTE read
uint32_t read_id = 0;        // Packet header <id> field
uint32_t read_size = 0;      // Packet header <len> field
uint32_t read_ctr = 0;       // Data bytes read
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
void debug_print(const char* fmt, ...);
void sbc_uart_send(const char* fmt, ...);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t test_str[8] = "testing!";
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
#define some_addr 2
uint8_t msg[3] = { 3, 5, 7 };
uint8_t something[2] = { 1, 2 };

/* USER CODE END 1 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize Peripherals */
	MX_GPIO_Init();
	MX_CAN_Init();
	// MX_SPI1_Init();
	MX_SPI2_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	// MX_TIM17_Init();

	MsgQueue_Init(&uart_txbuf);
	MsgQueue_Init(&uart_rxbuf);
	MsgQueue_Init(&can_rxbuf0);
	MsgQueue_Init(&can_rxbuf1);
	MsgQueue_Init(&udp_txbuf);
	MsgQueue_Init(&current_msg);

	HAL_Delay(500);

	init_udp();

	CAN_Begin(CAN_MODE_NORMAL); // Delay Between Initialising CAN and Starting CAN
	CAN_SetAllFilters();

	CAN_RecvStart();
	UART_RecvStart();

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 0
	debug_print("\r\n\nI am Logic\n\n\r");

	debug_print("INIT Completed\r\n");
	sbc_uart_send("INIT Completed\r\n");
#endif
#endif

	// SDCARD_Unselect();
	// int alive = SDCARD_Init();

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		// PingUART();

		/* USER CODE END WHILE */
		if (!MsgQueue_Empty(&uart_rxbuf)) {
			// uart_rxbuf -> can_txbuf
			ForwardToCAN_NewRoutine();
		}

		if (!MsgQueue_Empty(&can_rxbuf0) || !MsgQueue_Empty(&can_rxbuf1)) {
			// can_rxbuf -> udp_txbuf
			// can_rxbuf -> uart_txbuf
			ForwardFromCAN_Routine();
		}

		if (!MsgQueue_Empty(&uart_txbuf)) {
			// NOTE: Not actually TXing data; this is left to the UART TX interrupt
			UART_TranStart();
		}

		if (!MsgQueue_Empty(&can_txbuf)) {
			// can_txbuf ->
			CAN_TranStart();
		}

		if (!MsgQueue_Empty(&udp_txbuf)) {
			// udp_txbuf ->
			UDP_Routine();
		}

		if ((HAL_GetTick() - can_hb_timer) > CAN_HB_INTERVAL) {
			// -> uart_txbuf
			// -> can_txbuf
			SendHeartbeat();
		}

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 3
		if ((HAL_GetTick() - dbg_ping_timer) > DBG_PING_INTERVAL) {
			debug_print("logic alive ping");
			dbg_ping_timer = HAL_GetTick();
		}
#endif
#endif
	}
	/* USER CODE END 1 */
}

/**
 * @brief Print Messages on UART Monitor
 * @param String Format, Arguments ...
 * @retVal None
 */
void debug_print(const char* fmt, ...)
{
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int len = strlen(buffer);
	HAL_UART_Transmit(&huart1, (uint8_t*) buffer, len, -1);
}

void sbc_uart_send(const char* fmt, ...)
{
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	int len = strlen(buffer);
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, len, -1);
}

/**
 * @brief Test the latecncy between read and transmit to SBC
 *
 */
void PingUART()
{
	if (!MsgQueue_Empty(&uart_rxbuf)) {
		// Pop out the 2 start bytes
		MsgQueue_pop(&uart_rxbuf);
		MsgQueue_pop(&uart_rxbuf);

		uint8_t id = MsgQueue_pop(&uart_rxbuf);
		uint8_t len = MsgQueue_pop(&uart_rxbuf);
		if (id == 100) {
			MsgQueue_push(&uart_txbuf, 100);
			MsgQueue_push(&uart_txbuf, len);
			for (int i = 0; i < len; i++) {
				MsgQueue_push(&uart_txbuf, MsgQueue_pop(&uart_rxbuf));
			}
		}
		else {
			for (int i = 0; i < len; i++) {
				MsgQueue_pop(&uart_rxbuf);
			}
		}
	}
}

/**
 * @brief Handles forwarding UART messages to CAN bus and over UDP
 * @note Called from main thread context
 */

#define WAIT_FOR_END 0
#define PROCESS      1
// Framing of UART message
// |Next 0 index|CAN ID|Message Length|Message MAX 8 bytes...|Checksum 1|Checksum 2|0 delimeter|
#define MAX_UART_MSG_LEN 14

void ForwardToCAN_NewRoutine()
{
	static uint8_t state = WAIT_FOR_END;
	static uint8_t msg_len_tracker = 0;
	// don't need to store the delimiting 0
	static uint8_t msg_buffer[MAX_UART_MSG_LEN - 1];
	static bool bad_msg = false;
	uint8_t msg_cnt = 0;
	uint8_t byte_cnt = 0;
	uint8_t msgQueue_popIndex;

	// Process a maximum of 2 messages at one go / maximum of 2*MAX_UART_MSG_LEN bytes
	while (msg_cnt < 2 || byte_cnt < 2 * MAX_UART_MSG_LEN) {
		uint8_t isAvailable = !MsgQueue_Empty(&uart_rxbuf);
		UART_RecvPause();
		if (isAvailable) {
			msgQueue_popIndex = MsgQueue_getPopIndex(&uart_rxbuf);
			incoming_byte = MsgQueue_pop(&uart_rxbuf);
		}
		UART_RecvResume();

		// If UART buffer empty break from loop
		if (!isAvailable) {
			break;
		}

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
		debug_print("received byte: %u\n", incoming_byte);
#endif
#endif

		// track number off bytes processed in this loop instance
		byte_cnt++;

		// Check if a byte is dropped by UART IRQ handler
		// Handler dropes any byte that might be corrupted / because the buffer has overflowed
		if (dropped_byte) {
			if (msgQueue_popIndex == dropped_byte_index) {
				bad_msg = true;
				// unset dropped_byte bool
				dropped_byte = false;
			}
		}

		// STATE MACHINE to read and construct messages
		if (state == WAIT_FOR_END) {
			// check if 0x00 (stuffed byte) is received
			// 0x00 marks the end of a message
			if (incoming_byte == 0) {
				// if msg frame contains any corruption (short of byte/exceed expected msg length)
				// drop message
				if (!bad_msg) {
					state = PROCESS;
				}
				else {
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
					debug_print("bad msg");
#endif
#endif
				}
				bad_msg = false;
			}
			// continue storing message into buffer
			else {
				// check if the message length has exceeded maximum possible length
				// Minus one as delimeter 0 is not considered to be part of the message
				if (msg_len_tracker < MAX_UART_MSG_LEN - 1) {
					msg_buffer[msg_len_tracker++] = incoming_byte;
				}
				// flag error on current frame, discard previous data to prevent buffer overflow
				else {
					msg_len_tracker = 0;
					msg_buffer[msg_len_tracker++] = incoming_byte;
					bad_msg = true;
				}
			}
		}

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
		debug_print("msg_len: %u\n", msg_len_tracker);
#endif
#endif

		if (state == PROCESS) {
			// decode cobs message
			// decoded message doesn't have the 0 delimeter and the next 0 header
			// in the cobs encoding protocol
			uint8_t decoded_msg_buffer[MAX_UART_MSG_LEN - 2];
			cobs_decode_result decode_result;
			uint16_t computed_checksum;
			uint8_t can_id;
			uint8_t data_len;
			uint16_t checksum1, checksum2;

			decode_result = cobs_decode(decoded_msg_buffer, MAX_UART_MSG_LEN - 2, msg_buffer, msg_len_tracker);

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
			for (uint8_t i = 0; i < msg_len_tracker - 1; i++) {
				debug_print("decoded_msg_buffer: %u\n", decoded_msg_buffer[i]);
			}
#endif
#endif

			// compute the fletcher checksum of the message and compare it with the
			// embedded checksum in the message (last 2 bytes)
			computed_checksum = fletcher_get(decoded_msg_buffer, msg_len_tracker - 3);

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
			debug_print("computed checksum: %u\n", computed_checksum);
#endif
#endif

			checksum1 = (uint16_t) decoded_msg_buffer[msg_len_tracker - 3];
			checksum2 = (uint16_t) decoded_msg_buffer[msg_len_tracker - 2];

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
			debug_print(
				"checksum1: %u checksum2: %u, combined checksum: %u\n", checksum1, checksum2, ((checksum2 << 8))
			);
#endif
#endif

			// if checksum_valid write message to tx_buf
			if (computed_checksum == (checksum1 | checksum2 << 8)) {
				can_id = decoded_msg_buffer[0];
				data_len = decoded_msg_buffer[1];
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
				debug_print("checksum pass\n");
#endif
#endif
				// check the length of message data field corresponds with actual message length
				if (data_len == msg_len_tracker - 5) {
					MsgQueue_push(&can_txbuf, can_id);
					MsgQueue_push(&can_txbuf, data_len);
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
					debug_print("can_id: %u\n", can_id);
					debug_print("data_len: %u\n", data_len);
#endif
#endif
					for (uint8_t i = 0; i < data_len; i++) {
						MsgQueue_push(&can_txbuf, decoded_msg_buffer[i + 2]);
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
						debug_print("%u ", decoded_msg_buffer[i + 2]);
#endif
#endif
					}
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
					debug_print("\n");
					for (uint8_t i = 0; i < msg_len_tracker; i++) {
						MsgQueue_push(&uart_txbuf, msg_buffer[i]);
					}
					MsgQueue_push(&uart_txbuf, 0);

#endif
#endif
					msg_cnt++;
				}
			}
			// TODO: flag some kind of error if checksum not valid / send a status to sbc

			// reset msg_buffer after processing
			msg_len_tracker = 0;
			state = WAIT_FOR_END;
		}
	}
}

void ForwardToCAN_Routine()
{
	// TODO: Add reset when error is detected

	// Processes messages byte by byte from uart_rxbuf until empty
	// - Control states are in global variable so re-entering function can continue from last routine.
	while (1) {
		UART_RecvPause();
		uint8_t isAvailable = !MsgQueue_Empty(&uart_rxbuf);
		if (isAvailable) {
			incoming_byte = MsgQueue_pop(&uart_rxbuf);
		}
		UART_RecvResume();

		// Everything processed, exit routine
		if (!isAvailable) {
			break;
		}
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
		if (isAvailable) {
			debug_print("%u\n", incoming_byte);
		}
#endif
#endif

		/**
		 * LTH: This can potentially put us in an irrecoverable state of continuous framing errors
		 *
		 *
		 * Two possible issues:
		 * 1)   No validation that the start bytes are consecutive:
		 *      [ |5E|FE|xx|xx|FF|xx| ... ] [ ... ]
		 *        <----->          ^--- (now 2nd data byte is taken as length)
		 *      (start bytes)
		 *
		 *      Where:
		 *        [ ... ] denotes a packet
		 *        | ... | denotes a byte
		 *        xx denotes a byte value whose exact value is not important
		 * 2)   Start bytes can be present in data as unescaped values
		 *      - Drawing packet boundaries relies on length field
		 *      - Should byte stuff 0xFE in data so we can use two consecutive 0xFEs as a sync field
		 *
		 * Mitigations:
		 * 1) Better state machine that requires two _consecutive_ start bytes before parsing data
		 * 2) CAN standard mandates that we have at most 8 data bytes and this should be validated.
		 *    - Also a possible segfault cause: If your packet is misaligned and you read a data as a length field,
		 * any data with byte value >8 will do an out-of-bounds write to read_buffer. (Although I checked the
		 * firmware and such a write will corrupt only hspi1 (unused) and udp_txbuf and neither should cause any
		 * segfaults). Either way this may also wreck the can_txbuf causing misaligned frames to be sent on CAN 3)
		 * Drop the frame being received if there are any errors during transmission; better to not send than send a
		 * potentially dangerous command (?)
		 * 4) In the same vein, the UART-SBC protocol should at least have some
		 * checksum on the packet level; even an XOR checksum in the packet footer would be better than nothing
		 */

		// First start byte of packet
		if (incoming_byte == START_BYTE && read_start_ctr == 0) {
			read_start_ctr = 1;
			read_ctr = 1;
		}
		// Second start byte of packet
		else if (incoming_byte == START_BYTE && read_start_ctr == 1) {
			read_start_ctr++;
		}
		// Both start bytes read; bytes here are packet fields/data
		else if (read_start_ctr == 2) {
			// 3rd byte is ID
			if (read_ctr == 1) {
				read_id = incoming_byte;
				read_ctr++;
			}
			// 4th byte is data size
			else if (read_ctr == 2) {
				read_size = incoming_byte;
				read_ctr++;
				// TODO: Can do a sanity check here for (read_size <= 8) since that is max. len. of CAN frame
			}
			// Subsequent bytes are data
			else if (read_ctr > 2) {
				// store data in read buffer
				read_buffer[read_ctr - 3] = incoming_byte;
				// received full data
				if (read_ctr == (2 + read_size)) {
					// TODO: ret can be removed, code basically assumes msg_queues are never full
					uint8_t ret = 1;

					// Write to CAN TX Buffer (1 Success, 0 Fail)
					ret &= MsgQueue_push(&can_txbuf, read_id);

					ret &= MsgQueue_push(&can_txbuf, read_size);

					for (uint8_t i = 0; i < read_size; i++) {
						ret &= MsgQueue_push(&can_txbuf, read_buffer[i]);
					}

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 2
					debug_print("ID: %u ", read_id);
					debug_print("Size: %u ", read_size);
					for (uint8_t i = 0; i < read_size; i++) {
						debug_print("Buffer: %02x", read_buffer[i]);
					}
					debug_print("\n\r");
#endif
#endif

					// Reset counters
					read_start_ctr = 0;
					read_ctr = 0;

					// UART_TestIntegrity(read_id,read_buffer,read_size);
				}
				// Full message has not been read
				else {
					read_buffer[read_ctr - 3] = incoming_byte;
					read_ctr++;
				}
			}
		}
	}
}

/**
 * @brief CAN message filter for messages sent to SBC via UART
 *
 * @param id // CAN ID
 */
bool isMsgForSBC(uint8_t can_fifo_num, uint8_t filter_num)
{
	if ((can_fifo_num == 0 && (filter_num == 6 || filter_num == 7)) ||
	    (can_fifo_num == 1 && (filter_num == 0 || filter_num == 1)))
	{
		return true;
	}
	return false;
}

/**
 * @brief CAN message filter for messages sent to OCS via UDP
 *
 * @param id // CAN ID
 */
bool isMsgForOCS(uint8_t can_fifo_num, uint8_t filter_num)
{
	if (can_fifo_num == 1 && (filter_num == 0 || filter_num == 1)) {
		return true;
	}
	return false;
}

/**
 * @brief Handles forwarding CAN messages to SBC over UART and OCS over UDP
 * @note Called from main thread context
 */
void ForwardFromCAN_Routine()
{
	// Doesn't have next-0 header and 0 delimeter at the end
	uint8_t msg_buffer[MAX_UART_MSG_LEN - 2];
	uint8_t encoded_msg_buffer[MAX_UART_MSG_LEN];
	uint16_t fletcher_checksum;
	uint8_t raw_msg_len = 0, encoded_msg_len = 0;
	uint8_t id, filterID, len, temp;
	cobs_encode_result encode_result;
	// _ForwardToSBC_Routine(&can_rxbuf0);
	// // NOTE: Split into two critical sections so we don't ignore CAN interrupts for too long
	// _ForwardToSBC_Routine(&can_rxbuf1);

	if (!MsgQueue_Empty(&can_rxbuf0)) {
		CAN_RecvPause();
		id = MsgQueue_pop(&can_rxbuf0);
		filterID = MsgQueue_pop(&can_rxbuf0);
		len = MsgQueue_pop(&can_rxbuf0);

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 4
		debug_print("CAN0 (%d, %d, %d)\n", id, filterID, len);
#endif
#endif

		// Forward message to OCS and Build message for uart

		msg_buffer[raw_msg_len++] = id;
		msg_buffer[raw_msg_len++] = len;

		if (isMsgForOCS(0, filterID)) {
			MsgQueue_push(&udp_txbuf, id);
			MsgQueue_push(&udp_txbuf, len);
		}

		for (uint8_t i = 0; i < len; i++) {
			temp = MsgQueue_pop(&can_rxbuf0);

			if (isMsgForSBC(0, filterID)) {
				msg_buffer[raw_msg_len++] = temp;
			}

			if (isMsgForOCS(0, filterID)) {
				MsgQueue_push(&udp_txbuf, temp);
			}
		}

		CAN_RecvResume();

		if (isMsgForSBC(0, filterID)) {
			// Compute fletcher checksum
			fletcher_checksum = fletcher_get(msg_buffer, raw_msg_len);
			// can id + data len (up to 8B) + data + checksum (2B)

			// Append checksum at the end of message
			msg_buffer[raw_msg_len++] = (uint8_t) UINT8_MAX & fletcher_checksum;
			msg_buffer[raw_msg_len++] = (uint8_t) (fletcher_checksum >> 8);

			// Encode message with COTS
			encode_result = cobs_encode(encoded_msg_buffer, MAX_UART_MSG_LEN, msg_buffer, raw_msg_len);
			encoded_msg_len = (uint8_t) encode_result.out_len;

			// Append 0 at the end of encoded message as a delimeter
			encoded_msg_buffer[encoded_msg_len++] = 0;
		}

		UART_RecvPause();

		// Write encoded message into UART transmit buffer
		if (isMsgForSBC(0, filterID)) {
			for (int i = 0; i < encoded_msg_len; i++) {
				MsgQueue_push(&uart_txbuf, encoded_msg_buffer[i]);
			}
#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 1
			debug_print("encoded_msg_len: %u", encoded_msg_len);
			for (int i = 0; i < encoded_msg_len; i++) {
				debug_print("encoded_msg_buffer: %u\n", encoded_msg_buffer[i]);
			}
#endif
#endif
		}

		UART_RecvResume();
	}

	// NOTE: Split into two critical sections so we don't ignore CAN interrupts for too long
	raw_msg_len = 0;
	encoded_msg_len = 0;

	if (!MsgQueue_Empty(&can_rxbuf1)) {
		CAN_RecvPause();
		id = MsgQueue_pop(&can_rxbuf1);
		filterID = MsgQueue_pop(&can_rxbuf1);
		len = MsgQueue_pop(&can_rxbuf1);

#ifdef BB_DEBUG
#if DEBUG_LEVEL >= 4
		debug_print("CAN0 (%d, %d, %d)\n", id, filterID, len);
#endif
#endif

		// Forward message to OCS and Build message for uart

		msg_buffer[raw_msg_len++] = id;
		msg_buffer[raw_msg_len++] = len;

		if (isMsgForOCS(1, filterID)) {
			MsgQueue_push(&udp_txbuf, id);
			MsgQueue_push(&udp_txbuf, len);
		}

		for (uint8_t i = 0; i < len; i++) {
			temp = MsgQueue_pop(&can_rxbuf1);

			if (isMsgForSBC(1, filterID)) {
				msg_buffer[raw_msg_len++] = temp;
			}

			if (isMsgForOCS(1, filterID)) {
				MsgQueue_push(&udp_txbuf, temp);
			}
		}

		CAN_RecvResume();

		if (isMsgForSBC(1, filterID)) {
			// Compute fletcher checksum
			fletcher_checksum = fletcher_get(msg_buffer, raw_msg_len);
			// can id + data len (up to 8B) + data + checksum (2B)

			// Append checksum at the end of message
			msg_buffer[raw_msg_len++] = (uint8_t) UINT8_MAX & fletcher_checksum;
			msg_buffer[raw_msg_len++] = (uint8_t) (fletcher_checksum >> 8);

			// Encode message with COTS
			encode_result = cobs_encode(encoded_msg_buffer, MAX_UART_MSG_LEN, msg_buffer, raw_msg_len);
			encoded_msg_len = (uint8_t) encode_result.out_len;

			// Append 0 at the end of encoded message as a delimeter
			encoded_msg_buffer[encoded_msg_len++] = 0;
		}

		UART_RecvPause();

		// Write encoded message into UART trasnmit buffer
		for (int i = 0; i < encoded_msg_len; i++) {
			MsgQueue_push(&uart_txbuf, encoded_msg_buffer[i]);
		}

		UART_RecvResume();
	}
}

/**
 * @brief Start UART TX interrupt to send from UART message queue
 */
void UART_TranStart()
{
	// Enable TXE, which will process uart_txbuf in interrupt (See USART2_IRQHandler)
	huart2.Instance->CR1 |= USART_CR1_TXEIE;
}

void CAN_TranStart()
{
	uint32_t txmailbox;
	// configure TxHeader
	CAN_TxHeaderTypeDef txheader;
	txheader.StdId = MsgQueue_pop(&can_txbuf);
	txheader.ExtId = 0;
	txheader.DLC = MsgQueue_pop(&can_txbuf);
	txheader.IDE = CAN_ID_STD;
	txheader.RTR = CAN_RTR_DATA;

	uint8_t txdata[12];
	uint8_t ret = 1;
	ret &= txheader.DLC;
	for (uint8_t i = 0; i < txheader.DLC; i++) {
		txdata[i] = MsgQueue_pop(&can_txbuf);
		ret &= txdata[i];
	}
	if (ret == 0) {
		// while(1);
	}
	if (HAL_CAN_AddTxMessage(&hcan, &txheader, txdata, &txmailbox) != HAL_OK) {
		// while(1);
	}
}

/**
 * @brief Handles forwarding messages over UDP
 * @note Called from main thread context
 */
void UDP_Routine()
{
	CAN_RecvPause();

	if (!MsgQueue_Empty(&udp_txbuf)) {
		uint8_t id = MsgQueue_pop(&udp_txbuf);
		uint8_t len = MsgQueue_pop(&udp_txbuf);
		uint8_t data[8];

		for (uint8_t i = 0; i < len; i++) {
			data[i] = MsgQueue_pop(&udp_txbuf);
		}
		// NOTE: Not sure if perf. is degraded if this is put after UDP write
		CAN_RecvStart();

		uint8_t ret = write_udp_buffer(id, data, len);
	}
	else {
		CAN_RecvStart();
	}
}

void SendHeartbeat()
{
	can_hb_timer = HAL_GetTick();

	MsgQueue_push(&can_txbuf, BB_CAN_ID_HEARTBEAT);   // hb message id
	MsgQueue_push(&can_txbuf, 1);                     // len
	MsgQueue_push(&can_txbuf, BB_HEARTBEAT_ID_LOGIC); // sbc_can hb id
	// TODO: Is disabling TXE required if the interrupt is disabled through NVIC?
	//       Feel like NVIC disable is a superset of USART2 IRQ events so this shouldn't need to be here
	// 		 In any case, if this is actually needed, maybe move this TXEIE clear/set to UART_RecvPause/Resume
	// huart2.Instance->CR1 &= ~USART_CR1_TXEIE;
	CAN_RecvPause();
	MsgQueue_push(&uart_txbuf, 6);                     // hb message id
	MsgQueue_push(&uart_txbuf, BB_CAN_ID_HEARTBEAT);   // hb message id
	MsgQueue_push(&uart_txbuf, 1);                     // len
	MsgQueue_push(&uart_txbuf, BB_HEARTBEAT_ID_LOGIC); // sbc_can hb id
	MsgQueue_push(&uart_txbuf, 36);                    // hb message id
	MsgQueue_push(&uart_txbuf, 103);                   // hb message id
	MsgQueue_push(&uart_txbuf, 0);                     // sbc_can hb id
	// huart2.Instance->CR1 |= USART_CR1_TXEIE;
	CAN_RecvResume();

	MsgQueue_push(&udp_txbuf, BB_CAN_ID_HEARTBEAT);   // hb message id
	MsgQueue_push(&udp_txbuf, 1);                     // len
	MsgQueue_push(&udp_txbuf, BB_HEARTBEAT_ID_LOGIC); // sbc_can hb id
}

/**
 * @brief Start handling of UART RX interrupts
 * @note The UART interrupt modifies uart_rxbuf and uart_txbuf
 */
void UART_RecvStart()
{
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);

	// Disable UART for configuration
	huart2.Instance->CR1 &= ~USART_CR1_UE;

	// - USART_CR1 TE (TX enable) is not set here as the UART TX interrupt is used
	// to automatically send data from uart_txbuf (see UART_TranStart)
	// - USART_CR1 ONEBIT defaults to 3 samples/bit

	// Enable RXNE and ORE interrupt
	huart2.Instance->CR1 |= USART_CR1_RXNEIE;
	// Enable UART receiver
	huart2.Instance->CR1 |= USART_CR1_RE;
	// Enable UART
	huart2.Instance->CR1 |= USART_CR1_UE;

	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
 * @brief Stop handling of UART RX interrupts
 */
void UART_RecvPause()
{
	HAL_NVIC_DisableIRQ(USART2_IRQn);
}

/**
 * @brief Resumes handling of UART RX interrupts
 */
void UART_RecvResume()
{
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
 * @brief Stop handling of CAN RX interrupts
 */
void CAN_RecvPause()
{
	HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
}

/**
 * @brief Resumes handling of CAN RX interrupts
 */
void CAN_RecvResume()
{
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

void UART_TestIntegrity(uint8_t read_id, uint8_t* read_buffer, uint8_t read_size)
{
	// ensure integrity by checking thruster idle frames
	uint8_t ret = 1;
	if (read_size != 8)
		ret = 0;
	else if (read_id == 1) {
		uint8_t cmp[8] = { 244, 1, 204, 1, 220, 1, 212, 1 };
		for (uint8_t i = 0; i < 8; i++) {
			if (cmp[i] != read_buffer[i])
				ret = 0;
		}
	}
	else if (read_id == 0) {
		uint8_t cmp[8] = { 189, 1, 244, 1, 244, 1, 244, 1 };
		for (uint8_t i = 0; i < 8; i++) {
			if (cmp[i] != read_buffer[i])
				ret = 0;
		}
	}

	else {
		ret = 0;
	}

	if (ret == 0) {
		// while(1);
	}
}

/**
 * @brief Configure CAN filters for CAN IDs (hardware filter)
 */
void CAN_SetAllFilters()
{
	/*
	 * ID 0 ~ 17, 19, 21 ~ 31 goes to FIFO0
	 */

	// Filter 0 configuration (20Hz)
	//
	// Filter num 0: Accept all IDs < 16
	// ID:      0b00001111
	// Mask:    0b11110000
	//
	// Filter num 1: Accept ID 19
	// ID: 		  0b00010011
	// Mask:	  0b11111111
	//
	// Group:   None
	CAN_SetFilter(0x13, 0xFF, 0x0F, 0xF0, 0, CAN_RX_FIFO0);

	// Filter 1 configuration: (22Hz)
	//
	// Filter num 2: Accept IDs 20 ~ 23
	// ID: 		  0b00010111
	// Mask:	  0b11111100
	//
	// Filter num 3: Accept IDs 24 ~ 31
	// ID: 		  0b00011111
	// Mask:	  0b11111000
	//
	// Group:   None
	CAN_SetFilter(0x1F, 0xF8, 0x17, 0xFC, 1, CAN_RX_FIFO0);

	// Filter 2 configuration: (1Hz)
	//
	// Filter num 4: Accept ID 40
	// ID: 		  0x00101000
	// Mask:	  0x11111111
	//
	// Filter num 5: Accept ID 68
	// ID: 		  0x01000100
	// Mask:	  0x11111111

	// Group:   None
	CAN_SetFilter(0x44, 0xFF, 0x28, 0xFF, 2, CAN_RX_FIFO0);

	// Filter 3 configuration: (16Hz)
	//
	// Filter num 6: Accept IDs 16, 17
	// ID: 		  0b00010001
	// Mask:	  0b11111110
	//
	// Filter num 7: Accept IDs 40 ~ 47 (40 will be accepted by filter num 4 due to hardware priority)
	// ID: 		  0x00101111
	// Mask:	  0x11111000
	//
	// Group:   SBC
	CAN_SetFilter(0x2F, 0xF8, 0x11, 0xFE, 3, CAN_RX_FIFO0);

	/*
	 * ID 32 ~ 2047 (excluding 64, 65, 69, 88) goes to FIFO1
	 */

	// Filter 4 configuration:
	//
	// Filter num 0: Accept IDs 32 ~ 39
	// ID: 		  0x00100111
	// Mask:	  0x11111000
	//
	// Filter num 1: Accept 48 ~ 63
	// ID: 		  0x00111111
	// Mask:	  0x11110000
	//
	// Group:   SBC, OCS
	CAN_SetFilter(0x3F, 0xF0, 0x27, 0xF8, 4, CAN_RX_FIFO1);

	// Filter 5 configuration:
	//
	// Filter num 2: Accept IDs 66, 67, 70, 71
	// ID: 		  0x01000111
	// Mask:	  0x11111010
	//
	// Filter num 3: Accept ID 68 (repeated not used)
	// ID: 		  0x01000100
	// Mask:	  0x11111111
	//
	// Group:   None
	CAN_SetFilter(0x44, 0xFF, 0x47, 0xFA, 5, CAN_RX_FIFO1);

	// Filter 6 configuration:
	//
	// Filter num 4: Accept IDs 72 ~ 79
	// ID: 		  0x01001111
	// Mask:	  0x11111000
	//
	// Filter num 5: Accept IDs 80 ~ 95
	// ID: 		  0x01011111
	// Mask:	  0x11110000
	//
	// Group:   None
	CAN_SetFilter(0x5F, 0xF0, 0x4F, 0xF8, 6, CAN_RX_FIFO1);

	// Filter 7 configuration:
	// Filter num 6: Accept IDs 96 ~ 127
	// ID: 		  0x01111111
	// Mask:	  0x11100000
	//
	// Filter num 7: Accept IDs 128 ~ 255
	// ID: 		  0x11111111
	// Mask:	  0x10000000
	//
	// Group:   None
	CAN_SetFilter(0xFF, 0x80, 0x7F, 0xE0, 7, CAN_RX_FIFO1);
}

/**
 * @brief Start handling of CAN RX interrupts
 */
void CAN_RecvStart()
{
	HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);

	// start notification (Interrupt)
	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK) {
		Error_Handler();
	}

	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief CAN Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN_Init(void)
{
	/* USER CODE BEGIN CAN_Init 0 */
	hcan.Instance = CAN;
	/* USER CODE END CAN_Init 0 */

	/* USER CODE BEGIN CAN_Init 1 */
	hcan.Instance->MCR &= ~(1 << 16); // This line is needed for CAN to work in debug mode!!!!!!!
	/* USER CODE END CAN_Init 1 */
	hcan.Init.Prescaler = 3;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = DISABLE;
	hcan.Init.AutoWakeUp = DISABLE;
	hcan.Init.AutoRetransmission = DISABLE;
	hcan.Init.ReceiveFifoLocked = DISABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CAN_Init 2 */

	/* USER CODE END CAN_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{
	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{
	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	// TODO: Attempts to increase baudrate causes stm to hang
	//       when SBC starts to send messages (UART IRQ gets triggered)
	// huart2.Init.BaudRate = 460800;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{
	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void)
{
	/* USER CODE BEGIN SPI2_Init 0 */

	/* USER CODE END SPI2_Init 0 */

	/* USER CODE BEGIN SPI2_Init 1 */

	/* USER CODE END SPI2_Init 1 */
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;

	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;

	hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&hspi2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI2_Init 2 */

	/* USER CODE END SPI2_Init 2 */
}

/**
 * @brief TIM17 Initialization Function
 * @param None
 * @retval None
 */
// Use basic mode for TIM17
static void MX_TIM17_Init(void)
{
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 24; // 25M / (24+1) = 1MHz
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = 999; // 1M / 1000 = 1kHz
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
	htim17.Init.RepetitionCounter = 0x0000; // generate interrupt after n clock overflow/underflow
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim17) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_12, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin : PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// W5500 control pins
	/* Configure GPIO pin: PB0 PB1 PB12*/
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB3 PB4 PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB6 PB7 */
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_12, GPIO_PIN_SET);
	/* USER CODE END MX_GPIO_Init_2 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
