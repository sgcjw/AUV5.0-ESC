// ###################################################
//
// ____________________    _____    _____________   ____
// \______   \______   \  /  _  \  /   _____/\   \ /   /
//  |    |  _/|    |  _/ /  /_\  \ \_____  \  \   Y   /
//  |    |   \|    |   \/    |    \/        \  \     /
//  |______  /|______  /\____|__  /_______  /   \___/
//         \/        \/         \/        \/
//
// POPB (Power Hull Board) for ASV 4.0
//
//
// Written By: Steve
//
// Summary:
//  - Gets Battery Status
//  - Turns on both battery when one battery is off
//  - Turns off both battery when button is pressed
//  - Read temp and humidty from SHT Sensor
//  - Send Hard Kill Status
//  - Kill/Unkill ESCs based on message from Thruster Board
//  - PMIC for Rest Power
// ###################################################

#include <Adafruit_SHT4x.h>
#include <Arduino.h>
#include <INA238.h>
#include <Torqeedo.h>
#include <can.h>
#include <can_defines.h>
#include <math.h>

#include "POPB/define.h"
#include "POPB/main.h"
#include "POPB/button.h"

#include "common/bb_can.h"
using namespace bb;


//=========================
//
//		Global declarations
//
//=========================

// Button control variable
bool Button_Off = false;

// PMIC variable
uint8_t PMIC_Target = 0; // 0 for Rest, 1 for PC

/*CAN*/
// CAN variables
MCP_CAN can(CAN_CS);
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
uint32_t can_timers[NUM_CAN_TIMER_ID];

/*Battery*/
// Torqeedo battery instantiations
Torqeedo battery1(TORQEEDO1_RXEN, TORQEEDO1_DXEN, TORQEEDO1_ON, 1);
Torqeedo battery2(TORQEEDO2_RXEN, TORQEEDO2_DXEN, TORQEEDO2_ON, 2);
// Battery monitoring
uint32_t batt_update_timer = 0;
uint32_t batt_monitor_timers[NUM_BATT] = { 0 };
// Battery controlling
bool off_battery[NUM_BATT] = { 0 };
uint32_t batt_control_timers[NUM_BATT] = { 0 };
// Battery heartbeats
bool batt_heartbeats[NUM_BATT] = { 0 };
uint32_t batt_heartbeat_timers[NUM_BATT] = { 0 };

// PMIC instantiation
INA238_IIC ina238R(&Wire, INA238_I2C_ADDRESS1, 10, 0.016);
INA238_IIC ina238P(&Wire, INA238_I2C_ADDRESS3, 20, 0.004);

// Sensor Variables
Adafruit_SHT4x sht = Adafruit_SHT4x();
sensors_event_t humidity, temp;
int int_stats[STAT_COUNT] = { 0 };

// Sensor Callback Timers (Read)
uint32_t SHT4x_timers = 0;

// ESC Kill Flag
uint8_t unkill_flag = 0b00;

//======================
//
//		SETUP & LOOP
//
//=======================
void setup()
{
	Serial.begin(115200);
	Serial.println("POPB Initialising");
	// PMIC Init
	INA238_init();
	// Button Init
	button_init();
	// Sensors Init
	SHT_init();

	/*Contactor Init*/
	pinMode(PEN_1, OUTPUT);
	digitalWrite(PEN_1, HIGH);
	/*ISO_PC Control Init*/
	pinMode(ISO_OFF_PC, OUTPUT);
	digitalWrite(ISO_OFF_PC, LOW);
	/*CAN Init*/
	pinMode(CAN_CS, OUTPUT);
	digitalWrite(CAN_CS, HIGH);
	// Can Timers Init
	timer = millis();
	for (int i = 0; i < NUM_CAN_TIMER_ID; i++) {
		can_timers[i] = millis();
	}
	CAN_init();
	set_CANMask();

	/*Battery initialisations*/
	battery1.init();
	battery1.checkBatteryOnOff(); //initilise batt 1 stats communication
	battery2.init();
	battery2.checkBatteryOnOff(); //initilise batt 2 stats communication
	Serial.flush();

	// Battery Timer Init
	for (int i = 0; i < NUM_BATT; i++) {
		batt_monitor_timers[i] = millis();
	}
	batt_update_timer = millis();

	Serial.println("POPB Initialised successfully");
}

void loop()
{
	SHT_read();
	check_button();
	/*Battery Monitoring*/
	//battery1.checkBatteryOnOff();
	if (battery1.readMessage()) {
		batt_heartbeats[BATT1_IDX] = true;
		can_timers[BATT1_STATS_TIMER_IDX] = millis();
	}
	//battery2.checkBatteryOnOff();
	if (battery2.readMessage()) {
		batt_heartbeats[BATT2_IDX] = true;
		can_timers[BATT2_STATS_TIMER_IDX] = millis();
	}

	//request battery stats
	if ((millis() - batt_update_timer) > BATT_UPDATE_TIMEOUT) {
		battery1.checkBatteryConnected();
		battery1.requestUpdate();
		battery2.checkBatteryConnected();
		battery2.requestUpdate();
		batt_update_timer = millis();
	}

	//publish_CAN();
	reset_batteryheartbeat();
	check_CANmsg();
	off_onebattery();
}

void INA238_init()
{
	if (ina238R.begin() != true) {
		Serial.println("INA238R begin failed");
	}
	else if (ina238P.begin() != true){
	 	Serial.println("INA238P begin failed");
	}
	else {
		Serial.println("both INA238 begin successfully");
	}
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

		// switch (id) {
		// case BB_CAN_ID_CONTACTOR_CTRL:
		// 	if (can_rxbuf[0] == 1) { // if killed
		// 		digitalWrite(PEN_1, HIGH);
		// 	}
		// 	else {
		// 		digitalWrite(PEN_1, LOW);
		// 	}
		// break;

		// case BB_CAN_ID_BATT_CTRL:
		// 	if (can_rxbuf[1] == 1){ // battery 1 control
		// 		if (can_rxbuf[0] == 1){
		// 			battery1.offBattery();
		// 			batt_control_timers[0] = millis();
		// 			off_battery[0] = 1 ;
		// 			Serial.println("SW turning off Battery 1");
		// 		}
		// 		else{
		// 			battery1.offBattery();
		// 			delay(1000);
		// 			battery1.onBattery();
		// 			Serial.println("SW turn on Battery 1");
		// 		}
		// 	}
		// 	else { // battery 2 control
		// 		if (can_rxbuf[0] == 1){
		// 			battery2.offBattery();
		// 			batt_control_timers[1] = millis();
		// 			off_battery[1] = 1;
		// 			Serial.println("SW turning off Battery 2");
		// 		}
		// 		else{
		// 			battery2.offBattery();
		// 			delay(1000);
		// 			battery2.onBattery();
		// 			Serial.println("SW turn on Battery 2");
		// 		}
		// 	}
			
		// 	break;

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

/**
 * @brief publish POPB heartbeats and battery stats
 * @param None
 * @retval None
 */
void publish_CAN()
{
#if BB_DEBUG
	// Serial.println("POPB CAN Transmissions:");
#endif
	// Heartbeat
	if (millis() - can_timers[HEARTBEAT_TIMER_IDX] > CAN_HEARTBEAT_LOOP) {
#if BB_DEBUG
		Serial.println("1: Heartbeats");
#endif
		publishPOPB_heartbeats();
		can_timers[HEARTBEAT_TIMER_IDX] = millis();
	}
	// Hardkill
	if (millis() - can_timers[HARDKILL_TIMER_IDX] > CAN_HARDKILL_LOOP) {
#if BB_DEBUG
		Serial.println("2: Hardkill");
#endif
		publishCAN_Hardkill();
		can_timers[HARDKILL_TIMER_IDX] = millis();
	}
	// Battery 1 stats
	if (batt_heartbeats[BATT1_IDX] && (millis() - can_timers[BATT1_STATS_TIMER_IDX] > CAN_BATT1_STATS_LOOP)) {
#if BB_DEBUG
		Serial.println("3: Battery 1 Stats");
#endif
		publishBatt_stats(&battery1);
		can_timers[BATT1_STATS_TIMER_IDX] = millis();
	}
	// Battery 2 stats
	if (batt_heartbeats[BATT2_IDX] && (millis() - can_timers[BATT2_STATS_TIMER_IDX] > CAN_BATT2_STATS_LOOP)) {
#if BB_DEBUG
		Serial.println("4: Battery 2 Stats");
#endif
		publishBatt_stats(&battery2);
		can_timers[BATT2_STATS_TIMER_IDX] = millis();
	}
	// PMIC Rest stats
	if (millis() - can_timers[PMIC_REST_STATS_TIMER_IDX] > CAN_PMIC_REST_STATS_LOOP) {
		PMIC_Target = 0;
#if BB_DEBUG
		Serial.println("5: PMIC Rest Stats");
#endif
		publishPMIC_stats(PMIC_Target);
		can_timers[PMIC_REST_STATS_TIMER_IDX] = millis();
	}
		// PMIC PC stats
	if (millis() - can_timers[PMIC_PC_STATS_TIMER_IDX] > CAN_PMIC_PC_STATS_LOOP) {
		PMIC_Target = 1;
#if BB_DEBUG
		Serial.println("6: PMIC PC Stats");
#endif
		publishPMIC_stats(PMIC_Target);
		can_timers[PMIC_PC_STATS_TIMER_IDX] = millis();
	}
	// PH Stats
	if (millis() - can_timers[PH_STATS_TIMER_IDX] > CAN_PH_STATS_LOOP) {
#if BB_DEBUG
		Serial.println("7: PH Stats");
#endif
		publishPH_Stats();
		can_timers[PH_STATS_TIMER_IDX] = millis();
	}
}

/**
 * @brief publish heartbeat
 * @param heartbeat_id of the device
 * @retval None
 */

void publishCAN_heartbeat(uint8_t heartbeat_id)
{
	can_id_t id = can_id::HEARTBEAT;
	len = 1;
	can_txbuf[0] = { heartbeat_id };
	can.sendMsgBuf(id, 0, len, can_txbuf);
}

void publishCAN_Hardkill()
{
	can_id_t id = can_id::HARD_KILL;
	can_txbuf[0] = !digitalRead(ONBOARD_KILL);
	len = 1;
	can.sendMsgBuf(id, 0, len, can_txbuf);
}

/**
 * @brief publish POPB heartbeats (POPB + battery)
 * @param None
 * @retval None
 */
void publishPOPB_heartbeats()
{
	publishCAN_heartbeat(heartbeat_id::POPB);
	if (batt_heartbeats[BATT1_IDX]) {
		publishCAN_heartbeat(heartbeat_id::BATT1);
	}
	if (batt_heartbeats[BATT2_IDX]) {
		publishCAN_heartbeat(heartbeat_id::BATT2);
	};
}

/**
 * @brief publish battery stats (temperature, current, voltage)
 * @param pointer to Torqeedo battery
 * @param battery
 */
void publishBatt_stats(Torqeedo* battery)
{
#ifdef BB_DEBUG
	Serial.print("Battery:");
	Serial.println(battery->getbattNum());
	Serial.print("Voltage:");
	Serial.println(battery->getVoltage());
	Serial.print("Current:");
	Serial.println(0 - (int16_t) (battery->getCurrent() * 100));
	Serial.print("Temperature:");
	Serial.println(battery->getTemperature());
#endif
	len = 6;
	if (battery->getbattNum() == 1) {
		id = can_id ::BATT1_STATS;
	}
	else if (battery->getbattNum() == 2) {
		id = can_id ::BATT2_STATS;
	}
	can.setupCANFrame(can_txbuf, 0, 2, battery->getVoltage());
	can.setupCANFrame(can_txbuf, 2, 2, 0 - (int16_t) (battery->getCurrent()));
	can.setupCANFrame(can_txbuf, 4, 2, battery->getTemperature());
	can.sendMsgBuf(id, 0, len, can_txbuf);
}

void publishPMIC_stats(uint8_t PMIC_Target)
{
	if (PMIC_Target == 0) {
#ifdef BB_DEBUG
		Serial.println("To Rest");
		Serial.print("Voltage:");
		Serial.println(ina238R.getBusVoltage_V());
		Serial.print("Current:");
		Serial.println(ina238R.getCurrent_mA());
		Serial.print("Power:");
		Serial.println(ina238R.getPower_mW());
#endif
		len = 6;
		id = can_id ::POPB_REST_STATS;
		can.setupCANFrame(can_txbuf, 0, 2, (uint16_t) (ina238R.getBusVoltage_V()*10));
		can.setupCANFrame(can_txbuf, 2, 2, (uint16_t) ina238R.getCurrent_mA());
		can.setupCANFrame(can_txbuf, 4, 2, (uint16_t) ina238R.getPower_mW());
		can.sendMsgBuf(id, 0, len, can_txbuf);
	}
	else {
#ifdef BB_DEBUG
		Serial.println("To PC");
		Serial.print("Voltage:");
		Serial.println(ina238P.getBusVoltage_V());
		Serial.print("Current:");
		Serial.println(ina238P.getCurrent_mA());
		Serial.print("Power:");
		Serial.println(ina238P.getPower_mW());
#endif
		len = 6;
		id = can_id ::POPB_PC_STATS;
		can.setupCANFrame(can_txbuf, 0, 2, (uint16_t) (ina238R.getBusVoltage_V()*10));
		can.setupCANFrame(can_txbuf, 2, 2, (uint16_t) ina238P.getCurrent_mA());
		can.setupCANFrame(can_txbuf, 4, 2, (uint16_t) ina238P.getPower_mW());
		can.sendMsgBuf(id, 0, len, can_txbuf);
	}
}

// Publish power hull stats
void publishPH_Stats()
{
	id = can_id::POWERHULL_STATS;
	len = 4;

	can.setupCANFrame(can_txbuf, 0, 2, int_stats[0]);
	can.setupCANFrame(can_txbuf, 2, 2, int_stats[1]);

	can.sendMsgBuf(id, 0, len, can_txbuf);
}

//====================================
//
//		User-defined battery functions
//
//====================================

/*Heartbeats*/
void reset_batteryheartbeat()
{
#ifdef BB_DEBUG
	// Serial.println("Resetting heartbeat");
#endif
	// Turn off heartbeat if no batt response for 1s
	if ((millis() - batt_monitor_timers[BATT1_IDX]) > BATT_IDLE_TIMEOUT) {
		batt_heartbeats[BATT1_IDX] = false;
		// battery1.resetData();
		batt_monitor_timers[BATT1_IDX] = millis();
	}
	if ((millis() - batt_monitor_timers[BATT2_IDX]) > BATT_IDLE_TIMEOUT) {
		batt_heartbeats[BATT2_IDX] = false;
		// battery2.resetData();
		batt_monitor_timers[BATT2_IDX] = millis();
	}
}

void check_button()
{
	if (ReadButton() != 0) {
		battery1.offBattery();
		battery2.offBattery();
		Button_Off = true;
	}
	else {
		if (Button_Off) {
			battery1.onBattery();
			battery2.onBattery();
			Button_Off = false;
		}
	}
}

void off_onebattery()
{
	if (off_battery[0] == 1) {
		if (millis() - batt_control_timers[0] > BATT_CONTROL_TIMEOUT) {
			battery1.onBattery();
			off_battery[0] = 0;
			Serial.println("Battery 1 off");
		}
	}
	if (off_battery[1] == 1) {
		if (millis() - batt_control_timers[1] > BATT_CONTROL_TIMEOUT) {
			battery2.onBattery();
			off_battery[1] = 0;
			Serial.println("Battery 2 off");
		}
	}
}

//==========================================
//
//        SENSORS FUNCTIONS
//
//==========================================

void SHT_init()
{
	while (!sht.begin()) {
		delay(1);
	}
	sht.setPrecision(SHT4X_MED_PRECISION);
	sht.setHeater(SHT4X_NO_HEATER);

	Serial.println("SHT OK");
	Serial.print("Serial number 0x");
	Serial.println(sht.readSerial(), HEX);

	// Store initial values
	sht.getEvent(&humidity, &temp);
}

void SHT_read()
{
	if (millis() - SHT4x_timers > SHT_LOOP) {
		if (sht.getEvent(&humidity, &temp)) {
			// We only want 1dp precision
			int_stats[SHT_TEMP_IDX] = temp.temperature * 10;
			int_stats[SHT_HUMIDITY_IDX] = humidity.relative_humidity * 10;
			SHT4x_timers = millis();
			#ifdef BB_DEBUG
			// SHT precision 2dp for both temp and humidity
			// Serial.print("Temperature: ");
			// Serial.print(temp.temperature);
			// Serial.println(" *C");
			// Serial.print("Humidity: ");
			// Serial.print(humidity.relative_humidity);
			// Serial.println(" %");
			#endif // BB_DEBUG
		}
	}
}

void test_ESC()
{
	if (millis() - timer > 50) {
		vesc_duty = (1000 - 3200) / 3200.0;
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