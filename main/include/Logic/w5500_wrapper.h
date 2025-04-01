/**
  ******************************************************************************
  * File Name          : w5500_wrapper.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __UDP_H
#define __UDP_H

#include <stdint.h>

#define UDP_SOCKET              1

#define SBC_CAN_UDP_PORT        29141
#define OCS_UDP_PORT            29102 

#define ASV_VEH_ID              83

#define W5500_DEV 

  #ifdef W5500_DEV
  /* Pins for W5500 Module */
  #define W5500_CS_Pin            GPIO_PIN_0
  #define W5500_CS_GPIO_Port      GPIOB
  #define W5500_RST_Pin           GPIO_PIN_1
  #define W5500_RST_GPIO_Port     GPIOB 

  #else

  /* Pins for W5500 dev module */
  #define W5500_CS_Pin            GPIO_PIN_12
  #define W5500_CS_GPIO_Port      GPIOB
  #define W5500_RST_Pin           GPIO_PIN_13
  #define W5500_RST_GPIO_Port     GPIOC

  #endif

void init_udp();
uint8_t write_udp_buffer(uint8_t CAN_ID, uint8_t* data, uint8_t len);

#endif