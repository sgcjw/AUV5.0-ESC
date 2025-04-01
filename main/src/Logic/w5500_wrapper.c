/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stdarg.h"

#include "Logic/udp/socket.h"
#include "Logic/udp/wizchip_conf.h"
#include "Logic/w5500_wrapper.h"
#include "stm32f0xx_hal.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t SBC_CAN_MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
uint8_t SBC_CAN_IP[] = {192, 168, 1, 54};
uint8_t SBC_CAN_SUBNET[] = {255, 255, 255, 0};
uint8_t SBC_CAN_GATEWAY[] = {192, 168, 1, 1};
uint8_t SBC_CAN_DNS[] = {8, 8, 8, 8};

// uint8_t OCS_IP[4] = {192, 168, 1, 3};
uint8_t BROADCAST_IP[4] = {255, 255, 255, 255};

volatile bool ip_assigned = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
void W5500_Select(void);
void W5500_Unselect(void);
void W5500_ReadBuff(uint8_t* buff, uint16_t len);
void W5500_WriteBuff(uint8_t* buff, uint16_t len);
uint8_t W5500_ReadByte(void);
void W5500_WriteByte(uint8_t byte);
/* USER CODE END PFP */

void W5500_Select(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

void W5500_Unselect(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

void W5500_ReadBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Receive(&hspi2, buff, len, HAL_MAX_DELAY);
}

void W5500_WriteBuff(uint8_t* buff, uint16_t len) {
    HAL_SPI_Transmit(&hspi2, buff, len, HAL_MAX_DELAY);
}

uint8_t W5500_ReadByte(void) {
    uint8_t byte;
    W5500_ReadBuff(&byte, sizeof(byte));
    return byte;
}

void W5500_WriteByte(uint8_t byte) {
    W5500_WriteBuff(&byte, sizeof(byte));
}
#define test 0x02
void init_udp() {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);

    // Delay before reset
    uint8_t tmp = 0xFF;
    while (tmp--);

    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);

    // Register W5500 Callbacks
    reg_wizchip_cs_cbfunc(W5500_Select, W5500_Unselect);
    reg_wizchip_spi_cbfunc(W5500_ReadByte, W5500_WriteByte);
    reg_wizchip_spiburst_cbfunc(W5500_ReadBuff, W5500_WriteBuff);

    // Init W5500
    uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
    int init_status = wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

    // Write network information to W5500, Static IP
    wiz_NetInfo net_info = {
        .dhcp = NETINFO_STATIC
    };
    memcpy(net_info.mac, SBC_CAN_MAC, sizeof(SBC_CAN_MAC));
    memcpy(net_info.ip, SBC_CAN_IP, sizeof(SBC_CAN_IP));
    memcpy(net_info.gw, SBC_CAN_GATEWAY, sizeof(SBC_CAN_GATEWAY));
    memcpy(net_info.sn, SBC_CAN_SUBNET, sizeof(SBC_CAN_SUBNET));
    memcpy(net_info.dns, SBC_CAN_DNS, sizeof(SBC_CAN_DNS));
    wizchip_setnetinfo(&net_info);

    // Create network socket
    uint8_t code = socket(UDP_SOCKET, Sn_MR_UDP, SBC_CAN_UDP_PORT, 0);
    if(code != UDP_SOCKET) {
        return;
    }

}

uint8_t write_udp_buffer(uint8_t can_id, uint8_t* data, uint8_t len) {
    uint8_t frame_size = 3 + len;

    uint8_t buffer[frame_size];
    buffer[0] = ASV_VEH_ID;
    buffer[1] = can_id;
    buffer[2] = len;
    memcpy(buffer + 3, data, len);

    uint8_t ret = sendto(UDP_SOCKET, buffer, frame_size, BROADCAST_IP, OCS_UDP_PORT);
    
    return ret;
}