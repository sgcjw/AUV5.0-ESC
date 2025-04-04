// ###################################################
//
// ____________________    _____    _____________   ____
// \______   \______   \  /  _  \  /   _____/\   \ /   /
//  |    |  _/|    |  _/ /  /_\  \ \_____  \  \   Y   /
//  |    |   \|    |   \/    |    \/        \  \     /
//  |______  /|______  /\____|__  /_______  /   \___/
//         \/        \/         \/        \/
//
// Latency Test for VESC6
//
//
// Written By: Steve
//
// Summary:
// Run ESC to test signal latency
// ###################################################

#include <Arduino.h>
#include <can.h>
#include <can_defines.h>
#include <math.h>

#include "POPB/define.h"
#include "POPB/main.h"

#include "common/bb_can.h"
using namespace bb;


//=========================
//
//		Global declarations
//
//=========================

/*CAN*/
// CAN variables
MCP_CAN can(9);
uint8_t can_txbuf[8];
uint8_t can_rxbuf[8];
uint32_t id = 0;
uint8_t len = 0;

//For VESC testing
int32_t duty_cycle = 0;
float vesc_duty;
int vesc_duty_fb[8];
int vesc_curr_fb[8];
int32_t vesc_rpm_fb[8];
int vesc_voltage[8];

// CAN Callback Timers
uint32_t timer;

//======================
//
//		SETUP & LOOP
//
//=======================
void setup()
{
	Serial.begin(115200);
	Serial.println("Mega Initialising");
	/*CAN Init*/
	//pinMode(CAN_CS, OUTPUT);
	//digitalWrite(CAN_CS, HIGH);
	// Can Timers Init
	timer = millis();
	CAN_init();
	set_CANMask();
	Serial.println("Mega Initialised successfully");
}

void loop()
{
	test_ESC();
	//check_CANmsg();
}

//==============================
//
// CAN Initialisation Functions
//
//==============================

void CAN_init()
{
START_INIT:
	if (CAN_OK == can.begin(CAN_500KBPS)) // init can bus : baudrate = 1000Kbps
	{
#ifdef BB_DEBUG
		Serial.println("CAN BUS: OK");
#endif	
	}
	else {
#ifdef BB_DEBUG
		Serial.println("CAN BUS: FAILED");
		Serial.println("CAN BUS: Reinitializing");
#endif
		delay(1000);
		goto START_INIT;
	}
	Serial.println("INITIATING TRANSMISSION...");
}

void set_CANMask()
{
	// mask register 0
	//can.init_Mask(0, 0, can_mask::POWER_CONTROL.mask);
	//can.init_Filt(0, 0, can_mask::POWER_CONTROL.filter);
}

void check_CANmsg()
{
	if (CAN_MSGAVAIL == can.checkReceive()) {
		can.readMsgBufID(&id, &len, can_rxbuf); // read data,  len: data length, buf: data buf
		//For testing of VESC
		int esc_id = can.getCanId() & 0xFF;
		esc_id = esc_id - 1;
		//Serial.println(esc_id);
		int command = can.getCanId() >> 8 & 0xFF;
		switch (command) {
		case 9:
			vesc_rpm_fb[esc_id] = can.parseCANFrame(can_rxbuf, 0, 4);
			Serial.println(vesc_rpm_fb[esc_id]);
			//vesc_curr_fb[esc_id] = can.parseCANFrame(can_rxbuf, 4, 2);
			//Serial.println(vesc_curr_fb[esc_id]);
			//vesc_duty_fb[esc_id] = can.parseCANFrame(can_rxbuf, 6, 2);
			//Serial.println(vesc_duty_fb[esc_id]);
			break;
		case 27:
			//vesc_voltage[esc_id] = can.parseCANFrame(can_rxbuf, 4, 2);
			//Serial.println(vesc_voltage[esc_id]);
			break;
		default:
			break;
		}
		can.clearMsg();
	}
}

//=============================
//
//	CAN Transmission Functions
//
//=============================

void test_ESC()
{
	if (millis() - timer > 10) {
		vesc_duty = 0.2;
		duty_cycle = int32_t(vesc_duty * 100000);
		can_txbuf[0] = (duty_cycle >> 24) & 0xFF;
		can_txbuf[1] = (duty_cycle >> 16) & 0xFF;
		can_txbuf[2] = (duty_cycle >> 8) & 0xFF;
		can_txbuf[3] = (duty_cycle) &0xFF;
		// Send to vesc_id, extended frame, 4 bytes of data
		can.sendMsgBuf((0 << 8 | 7 ), 1, 4, can_txbuf);
		timer = millis();
	}
}