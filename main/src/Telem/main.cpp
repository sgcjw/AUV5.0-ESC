// ###################################################
//
// ____________________    _____    _____________   ____
// \______   \______   \  /  _  \  /   _____/\   \ /   /
//  |    |  _/|    |  _/ /  /_\  \ \_____  \  \   Y   /
//  |    |   \|    |   \/    |    \/        \  \     /
//  |______  /|______  /\____|__  /_______  /   \___/
//         \/        \/         \/        \/
//
// Telemetry and RC Board for ASV 4.0
//
// Written By: DJ & ZY
//
// Summary:
// - RC control of ASV
// - Display Telemetry of ASV (Telem Screen & RC Screen)
//
// ###################################################

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <can.h>
#include "Arduino_CRSF.h"
#include "LCD_Driver.h"
#include <Adafruit_RA8875.h>

#include "common/bb_can.h"
using namespace bb;

#include "Telem/define.h"
#include "Telem/main.h"

void setup()
{
	Serial.begin(115200);
	Serial.println("I'm Telem, bitches!");

	// CS screen and CAN GPIO init
	pinMode(SCREEN_CS, OUTPUT);
	digitalWrite(SCREEN_CS, HIGH);
	pinMode(CAN_CS, OUTPUT);
	digitalWrite(CAN_CS, HIGH);

	// LED GPIO init
	pinMode(RSSI_LED1, OUTPUT);
	pinMode(RSSI_LED2, OUTPUT);
	pinMode(RSSI_LED3, OUTPUT);
	pinMode(RSSI_LED4, OUTPUT);
	pinMode(GENERAL_LED1, OUTPUT);
	pinMode(GENERAL_LED2, OUTPUT);

	digitalWrite(RSSI_LED1, LOW);
	digitalWrite(RSSI_LED2, LOW);
	digitalWrite(RSSI_LED3, LOW);
	digitalWrite(RSSI_LED4, LOW);
	digitalWrite(GENERAL_LED1, LOW);
	digitalWrite(GENERAL_LED2, LOW);

	// Init screen
	screen_exist = screen.screen_init();
	if (screen_exist) {
		prepareScreen();
		Serial.println("Screen OK");
	}

	// Init CAN
	initCAN();
	Serial.println("CAN OK");
	setCANMask();

	// Init RC
	crsf.begin(&Serial3, 115200); // for receiver
	if (!crsf.isConnected()) {
		digitalWrite(GENERAL_LED2, crsf.isConnected());
		Serial.println("RC OK");
	}
}

void loop()
{
	/* RC Receive --------------------------------------------------------------------------------------- */
	rssiLeds(crsf.getRxRSSI(), crsf.isConnected());

	if (crsf.isConnected()) {
		// RC is connected, update RC heartbeat
		hbStats.rcHB.hb = true;
		hbStats.rcHB.lastReceived = millis();

		// get RC Kill and Vehicle Mode
		rcKill = rcGetKill();
		rcVehMode = rcGetVehicleControlMode();

		// Receive and update variable which is used for controlling what get published
		rcGimbalMode = rcGetGimbalMode();
		rcActLock = rcGetActLock();

		// Receive and update variable which is then used for CAN publish
		rcThrustDeploy = rcGetThrustDeploy();
		rcHydroDeploy = rcGetHydroDeploy();
		rcLoadShoot = rcGetLoadShoot();
		rcActCal = rcGetActCal();
		rcThrustAzi = rcGetThrustAziMode();
		rcMHPBCycle = rcGetMHPBCycle();

		if (rcGimbalMode == RC_Gimbal_Mode::THRUSTERS_MODE) {
			rcGetThrusterVal(rcCanPortBow, rcCanStarBow, rcCanPortQtr, rcCanStarQtr);

			rcCanPan = rc_ballshooter_ctrl::PAN_IGNORE;
			rcCanTilt = rc_ballshooter_ctrl::TILT_IGNORE;
			rcCanFlywheel = rc_ballshooter_ctrl::FLYWHEEL_IGNORE;
		}
		else {
			rcGetBallshooterVal(rcCanPan, rcCanTilt, rcCanFlywheel);

			rcCanPortBow = thruster_ctrl::STOP;
			rcCanStarBow = thruster_ctrl::STOP;
			rcCanPortQtr = thruster_ctrl::STOP;
			rcCanStarQtr = thruster_ctrl::STOP;
		}

		// set RC kill status
		if (rcKill == RC_Kill::KILLED) {
			killStats.rcKill = RC_Kill::KILLED;
		}
		else {
			killStats.rcKill = RC_Kill::OPERATIONAL;
		}
	}
	else {
		// RC is disconnected

		// set to KILLED
		rcKill = RC_Kill::KILLED;
	}

	/* RC Trasnmit --------------------------------------------------------------------------------------- */

	if (millis() - rcTelemLastSend > RCTELEMLOPP) {
		// telem debug values
		// mainHullClimateStats.isValid = true;
		// powerHullClimateStats.isValid = true;
		// actHullClimateStats.isValid = true;
		// navHullClimateStats.isValid = true;

		// batt1Stats.isValid = true;
		// batt2Stats.isValid = true;

		// mhpbStats.isValid = true;

		// popbPcStats.isValid = true;
		// popbRestStats.isValid = true;

		// pcTempStats.isValid = true;
		// ballshooterStats.isValid = true;
		// vertActStats.isValid = true;
		// thrusterAziStats.isValid = true;

		// // test values set 1
		// hbStats.pcHB.hb = false;
		// hbStats.lbkpHB.hb = false;
		// hbStats.mhpbHB.hb = true;
		// hbStats.thHB.hb = false;
		// hbStats.rcHB.hb = false;
		// hbStats.popbHB.hb = false;
		// hbStats.esc1HB.hb = false;
		// hbStats.esc2HB.hb = false;
		// hbStats.batt1HB.hb = false;
		// hbStats.batt2HB.hb = false;
		// hbStats.actHB.hb = false;
		// hbStats.navHB.hb = false;

		// batt1Stats.battVolt = 298;  // in dV
		// batt1Stats.battCurr = 2000; // in dA
		// batt1Stats.battTemp = 695;  // in d°C

		// batt2Stats.battVolt = 264;  // in dV
		// batt2Stats.battCurr = 1689; // in dA
		// batt2Stats.battTemp = 456;  // in d°C

		// popbPcStats.popbChVolt = 244;   // in dV
		// popbPcStats.popbChCurr = 38800; // in mA

		// popbRestStats.popbChVolt = 240;   // in dV
		// popbRestStats.popbChCurr = 23410; // in mA

		// mhpbStats.mhpbEn = 0x03;
		// mhpbStats.mhpbCycle = 0x01;
		// mhpbStats.mhpbFault = 0x01;

		// mainHullClimateStats.temp = 487; // in d°C
		// mainHullClimateStats.hum = 698;  // in d%

		// powerHullClimateStats.temp = 900;// in d°C
		// powerHullClimateStats.hum = 452;// in d%

		// actHullClimateStats.temp = 345;// in d°C
		// actHullClimateStats.hum = 987;// in d%

		// navHullClimateStats.temp = 769;// in d°C
		// navHullClimateStats.hum = 102;// in d%

		// pcTempStats.cpuTemp = 50;// in °C
		// pcTempStats.gpuTemp = 60;// in °C

		// thrusterAziStats.portBow = 0x0009;
		// thrusterAziStats.portBow = 0x0000;
		// thrusterAziStats.portQtr = 0xF000;
		// thrusterAziStats.starBow = 0x01;
		// thrusterAziStats.starQtr = 0xFFFF;

		// vertActStats.portBow = 0x06;
		// vertActStats.portQtr = 0x69;
		// vertActStats.starBow = 0x09;
		// vertActStats.starQtr = 0x50;
		// vertActStats.hydrophone = 0x01;

		// ballshooterStats.pan = 0xFF00;
		// ballshooterStats.tilt = 0x0;
		// ballshooterStats.flywheel = 0xFFFF;
		// ballshooterStats.loader = 0x01;

		// killStats.hardKill = RC_Kill::OPERATIONAL;
		// killStats.pcKill = RC_Kill::OPERATIONAL;
		// killStats.noTelemHBKill = RC_Kill::OPERATIONAL;

		// // set 2
		// hbStats.pcHB.hb = true;
		// hbStats.lbkpHB.hb = true;
		// hbStats.mhpbHB.hb = true;
		// hbStats.thHB.hb = true;
		// hbStats.rcHB.hb = true;
		// hbStats.popbHB.hb = true;
		// hbStats.esc1HB.hb = true;
		// hbStats.esc2HB.hb = true;
		// hbStats.batt1HB.hb = true;
		// hbStats.batt2HB.hb = true;
		// hbStats.actHB.hb = true;
		// hbStats.navHB.hb = true;

		// batt1Stats.battVolt = 264;   // in dV
		// batt1Stats.battCurr = 20000; // in cA
		// batt1Stats.battTemp = 1000;  // in d°C

		// batt2Stats.battVolt = 298;   // in dV
		// batt2Stats.battCurr = 20000; // in cA
		// batt2Stats.battTemp = 1000;  // in d°C

		// popbPcStats.popbChVolt = 25;    // in V
		// popbPcStats.popbChCurr = 40000; // in mA

		// popbRestStats.popbChVolt = 25;    // in V
		// popbRestStats.popbChCurr = 40000; // in mA

		// mhpbStats.mhpbEn = 0x00;
		// mhpbStats.mhpbCycle = 0x00;
		// mhpbStats.mhpbFault = 0x00;

		// mainHullClimateStats.temp = 1000; // in d°C
		// mainHullClimateStats.hum = 1000;  // in d%

		// powerHullClimateStats.temp = 1000; // in d°C
		// powerHullClimateStats.hum = 1000;  // in d%

		// actHullClimateStats.temp = 1000; // in d°C
		// actHullClimateStats.hum = 1000;  // in d%

		// navHullClimateStats.temp = 1000; // in d°C
		// navHullClimateStats.hum = 1000;  // in d%

		// pcTempStats.cpuTemp = 100; // in °C
		// pcTempStats.gpuTemp = 100; // in °C

		// // thrusterAziStats.portBow = 0x0001;
		// thrusterAziStats.portBow = 0x0002;
		// thrusterAziStats.portQtr = 0x0003;
		// thrusterAziStats.starBow = 0x0004;
		// thrusterAziStats.starQtr = 0xFFFF;

		// vertActStats.portBow = 0xFF;
		// vertActStats.portQtr = 0x04;
		// vertActStats.starBow = 0xFF;
		// vertActStats.starQtr = 0x01;
		// vertActStats.hydrophone = 0x50;

		// ballshooterStats.pan = 0x00000003;
		// ballshooterStats.tilt = 0x00000004;
		// ballshooterStats.flywheel = 0x0002;
		// ballshooterStats.loader = 0x02;

		// killStats.hardKill = RC_Kill::KILLED;
		// killStats.pcKill = RC_Kill::OPERATIONAL;
		// killStats.noTelemHBKill = RC_Kill::OPERATIONAL;

		// send telemetry data to RC
		hackyWhackyTelemetry(hbStats,
		                     batt1Stats,
		                     batt2Stats,
		                     popbPcStats,
		                     popbRestStats,
		                     mhpbStats,
		                     mainHullClimateStats,
		                     powerHullClimateStats,
		                     actHullClimateStats,
		                     navHullClimateStats,
		                     pcTempStats,
		                     thrusterAziStats,
		                     vertActStats,
		                     ballshooterStats,
		                     killStats,
		                     rcVehMode);
		rcTelemLastSend = millis();
	}

	/* Screen --------------------------------------------------------------------------------------- */
	// update screen every 1s
	// TBD how long screen update takes so to better optimise RC receiving
	if (screen_exist && (millis() - screen_prev_update) > SCREEN_LOOP) {
		updateHeartbeat();
		updateScreenData();
		screen_prev_update = millis();
	}

	/* CAN --------------------------------------------------------------------------------------- */
	// publish Telem Heartbeat at 2 Hz
	if (millis() - self_heartbeat_prev_update > can_freq::HEARTBEAT) {
		publishCANHeartbeat(heartbeat_id::TELEM_BOARD);
		self_heartbeat_prev_update = millis();
	}

	// publish kill status at 2 Hz
	if ((millis() - can_kill_prev_send) > can_freq::KILL_STATUS) {
		publishKill(rcKill);
		can_kill_prev_send = millis();
	}

	// publish control link status at 2 Hz
	if ((millis() - can_ctrl_link_prev_send) > can_freq::CTRL_LINK) {
		publishCtrlLink(rcVehMode, crsf.getRxRSSI());
		can_ctrl_link_prev_send = millis();
	}

	// publish MHPB cycle at 1 Hz and only if its RC control mode to avoid butter fingers during autonomous runs
	// on MHPB side, it will take in command from CAN from both for cycling request regardless of control mode
	if ((millis() - can_mhpb_prev_send) > can_freq::RC_MHPB_POWER_CTRL && rcVehMode == RC_Vehicle_Mode::RC_CTRL_MODE) {
		publishMHPBCycle(rcMHPBCycle);
		can_mhpb_prev_send = millis();
	}

	// only if OPERATIONAL that other messages get publish
	// other sub systems like thrusters and actuation ignores CAN commands if KILLED. multiple failsafes
	if (rcKill == RC_Kill::OPERATIONAL) {
		// publish RC thruster command at 20 Hz
		if ((millis() - can_thrusters_prev_send) > can_freq::RC_THRUSTER) {
			publishThrusters(rcCanPortBow, rcCanPortQtr, rcCanStarBow, rcCanStarQtr);
			can_thrusters_prev_send = millis();
		}

		// Actuation lock, if lock, all actuation doesnt get published
		if (rcActLock == RC_Act_Lock::UNLOCKED) {
			// publish thrusters and hydrophone deployment at 1 Hz
			if ((millis() - can_thrust_hydro_deploy_prev_send) > can_freq::RC_ACTUATION_VERT) {
				publishThrustHydroDeploy(rcThrustDeploy, rcHydroDeploy, rcActCal);
				can_thrust_hydro_deploy_prev_send = millis();
			}

			// publish thruster azimuth at 20 Hz
			if ((millis() - can_thrust_azi_prev_send) > can_freq::RC_THRUSTER_AZI) {
				publishThrustAziMode(rcThrustAzi, rcActCal);
				can_thrust_azi_prev_send = millis();
			}

			// publish ballshooter at 20 Hz
			if ((millis() - can_balls_shooter_prev_send) > can_freq::RC_BALLSHOOTER) {
				publishBallShooter(rcCanPan, rcCanTilt, rcCanFlywheel, rcLoadShoot, rcActCal);
				can_balls_shooter_prev_send = millis();
			}
		}
	}
	checkCANMsg();
	resetStats();

	/* Debug Print --------------------------------------------------------------------------------------- */
#if BB_DEBUG
	static unsigned long last_debug_print = millis();

	if (millis() - last_debug_print > 1000) {
		char txBuff[300];

		// Channels headers and labels
		sprintf(txBuff, "CH:\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\r\n");
		Serial.print(txBuff);

		if (rcGetGimbalMode() == RC_Gimbal_Mode::THRUSTERS_MODE) {
			sprintf(
				txBuff,
				"Name:\tYaw\tSurge\tSway\tFly\tKill\tVehMode\tT Dpl\tH Dpl\tActLock\tLdSht\tActCal\tMHPB Cy\tT Azi\tGimMode\tNONE\tNONE\r\n");
		}
		else {
			sprintf(
				txBuff,
				"Name:\tPan\tTilt\tSway\tFly\tKill\tVehMode\tT Dpl\tH Dpl\tActLock\tLdSht\tActCal\tMHPB Cy\tT Azi\tGimMode\tNONE\tNONE\r\n");
		}

		// Raw channels value from RC
		Serial.print(txBuff);
		sprintf(txBuff,
		        "Raw:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t\tlast received: %lu ms\r\n",
		        crsf.getChannel(YAW_PAN_CH),
		        crsf.getChannel(SURGE_TILT_CH),
		        crsf.getChannel(SWAY_CH),
		        crsf.getChannel(FLYWHEEL_CH),
		        crsf.getChannel(KILL_CH),
		        crsf.getChannel(VEHICLE_MODE_CH),
		        crsf.getChannel(THRUST_DEPLOY_CH),
		        crsf.getChannel(HYDRO_DEPLOY_CH),
		        crsf.getChannel(ACT_LOCK_CH),
		        crsf.getChannel(LOAD_SHOOT_CH),
		        crsf.getChannel(ACT_CAL_CH),
		        crsf.getChannel(MHPB_CYCLE_CH),
		        crsf.getChannel(THRUST_AZI_CH),
		        crsf.getChannel(GIMBAL_MODE_CH),
		        crsf.getChannel(15),
		        crsf.getChannel(16),
		        crsf.getRCChannelLastReceived());
		Serial.print(txBuff);

		Serial.println();

		// Link stats from RC
		sprintf(
			txBuff,
			"Stats:\tRxRSSI1: %d dBm  RxRSSI2: %d dBm  ActAnt: %d   RxRSSI: %d dBm  RxLQ: %d %%  RxSNR: %d dB  TxRSSI: %d dBm  TxLQ: %d %%  TxSNR: %d dB  RF mode: %d\tlast received: %lu ms\r\n",
			crsf.getRxRSSI1(),
			crsf.getRxRSSI2(),
			crsf.getActAnt(),
			crsf.getRxRSSI(),
			crsf.getRxLinkQuality(),
			crsf.getRxSNR(),
			crsf.getTxRSSI(),
			crsf.getTxLinkQuality(),
			crsf.getTxSNR(),
			crsf.getRFMode(),
			crsf.getLinkStatsLastReceived());
		Serial.print(txBuff);

		crsf.isConnected() ? Serial.println("Connected") : Serial.println("Disconnected");

		Serial.println();

		// Kill, Vehicle Mode, Gimbal Mode and Actuation lock
		sprintf(txBuff,
		        "Kill: %s\tVehMode: %s\t\tGimMode: %s\tActLock: %s\r\n",
		        rcGetKill() == RC_Kill::OPERATIONAL ? "OPERATIONAL" : "KILLED\t",
		        rcGetVehicleControlMode() == RC_Vehicle_Mode::RC_CTRL_MODE   ? "RC_CTRL_MODE"
		        : rcGetVehicleControlMode() == RC_Vehicle_Mode::STATION_KEEP ? "STATION_KEEP"
		        : rcGetVehicleControlMode() == RC_Vehicle_Mode::AUTONOMOUS   ? "AUTONOMOUS"
		                                                                     : "UNKNOWN",
		        rcGimbalMode == RC_Gimbal_Mode::THRUSTERS_MODE     ? "THRUSTERS_MODE\t"
		        : rcGimbalMode == RC_Gimbal_Mode::BALLSHOOTER_MODE ? "BALLSHOOTER_MODE"
		                                                           : "UNKNOWN",
		        rcActLock == RC_Act_Lock::LOCKED     ? "LOCKED"
		        : rcActLock == RC_Act_Lock::UNLOCKED ? "UNLOCKED"
		                                             : "UNKNOWN");
		Serial.print(txBuff);

		Serial.println();

		// Thrusters
		sprintf(txBuff,
		        "rcCanPortBow: %d\t\trcCanStarBow: %d\t\trcCanPortQtr: %d\t\trcCanStarQtr: %d\r\n",
		        rcCanPortBow,
		        rcCanStarBow,
		        rcCanPortQtr,
		        rcCanStarQtr);
		Serial.print(txBuff);

		Serial.println();

		// Calibration trigger,Calibration Selection, Thrust & Hydro Deployment abd Thruster Azimuth
		sprintf(txBuff,
		        "ActCal: %s\t\tT Dpl: %s\tH Dpl: %s\tT Azi: %s\r\n",
		        rcActCal == RC_Act_Cal::IDLE      ? "IDLE"
		        : rcActCal == RC_Act_Cal::PB_AZI  ? "PB_AZI"
		        : rcActCal == RC_Act_Cal::PQ_AZI  ? "PQ_AZI"
		        : rcActCal == RC_Act_Cal::SB_AZI  ? "SB_AZI"
		        : rcActCal == RC_Act_Cal::SQ_AZI  ? "SQ_AZI"
		        : rcActCal == RC_Act_Cal::PAN     ? "PAN"
		        : rcActCal == RC_Act_Cal::TILT    ? "TILT"
		        : rcActCal == RC_Act_Cal::PB_VERT ? "PB_VERT"
		        : rcActCal == RC_Act_Cal::PQ_VERT ? "PQ_VERT"
		        : rcActCal == RC_Act_Cal::SB_VERT ? "SB_VERT"
		        : rcActCal == RC_Act_Cal::SQ_VERT ? "SQ_VERT"
		        : rcActCal == RC_Act_Cal::HY_VERT ? "HY_VERT"
		                                          : "UNKNOWN",
		        rcGetThrustDeploy() == RC_Thrust_Deployment::DEPLOYED    ? "DEPLOYED\t"
		        : rcGetThrustDeploy() == RC_Thrust_Deployment::RETRACTED ? "RETRACTED"
		                                                                 : "UNKNOWN",
		        rcGetHydroDeploy() == RC_Hydro_Deployment::DEPLOYED    ? "DEPLOYED\t"
		        : rcGetHydroDeploy() == RC_Hydro_Deployment::RETRACTED ? "RETRACTED"
		                                                               : "UNKNOWN",
		        rcGetThrustAziMode() == RC_Thruster_Azi::STATION_KEEP ? "STATION_KEEP"
		        : rcGetThrustAziMode() == RC_Thruster_Azi::UNDER_WAY  ? "UNDER_WAY"
		                                                              : "UNKNOWN");
		Serial.print(txBuff);

		Serial.println();

		// Ballshooter
		sprintf(txBuff,
		        "Pan: %u\t\tTilt: %u\t\tFlywheel: %u\t\tLoadShoot: %s\r\n",
		        rcCanPan,
		        rcCanTilt,
		        rcCanFlywheel,
		        rcLoadShoot == RC_Load_Shoot::IDLE    ? "IDLE"
		        : rcLoadShoot == RC_Load_Shoot::LOAD  ? "LOAD"
		        : rcLoadShoot == RC_Load_Shoot::SHOOT ? "SHOOT"
		                                              : "UNKNOWN");
		Serial.print(txBuff);

		last_debug_print = millis();
	}
#endif
}

//===========================================
//
//        CAN FUNCTIONS
//
//===========================================

/**
 * @brief initialise CAN bus
 * @param None
 * @retval None
 */
void initCAN()
{
	// START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS)) { // init can bus : baudrate = 1000k
#if BB_DEBUG
		Serial.println("CAN init ok!");
#endif
	}
	else {
#if BB_DEBUG
		Serial.println("CAN init fail");
		Serial.println("Init CAN again");
		delay(1000);
#endif
		// goto START_INIT;
	}
}

/**
 * @brief set self CAN mask and filter
 * @param None
 * @retval None
 */
void setCANMask()
{
	CAN.init_Mask(0, 0, can_mask::TELEMETRY.mask);
	CAN.init_Filt(0, 0, can_mask::TELEMETRY.filter);
}

/**
 * @brief check for CAN message store it according to CAN Standards
 * @param None
 * @retval None
 */
void checkCANMsg()
{
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&CAN_msg_id, &CAN_msg_len, CAN_msg_buf); // read data,  len: data length, buf: data buf

		//         // #ifdef BB_DEBUG
		// 		Serial.print("Can id: ");
		// 		Serial.println(CAN.getCanId(), HEX);
		// 		Serial.print("Can Data: ");
		// 		for (int i = 0; i < CAN_msg_len; i++) {
		// 		  Serial.print(CAN_msg_buf[i]);
		// 		  Serial.print(" ");
		// 		}
		// 		Serial.println();
		// // #endif

		switch (CAN.getCanId()) {
		case can_id::MAINHULL_STATS: {
			mainHullClimateStats.temp = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			mainHullClimateStats.hum = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			mainHullClimateStats.lastReceived = millis();
			break;
		}
		case can_id::HEARTBEAT: {
			uint8_t device = CAN.parseCANFrame(CAN_msg_buf, 0, 1);
			// #ifdef BB_DEBUG
			// 			Serial.print("heartbeat: ");
			// 			Serial.println(device);
			// #endif

			if (device == heartbeat_id::PC) {
				hbStats.pcHB.hb = true;
				hbStats.pcHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::LOGIC) {
				hbStats.lbkpHB.hb = true;
				hbStats.lbkpHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::MHPB) {
				hbStats.mhpbHB.hb = true;
				hbStats.mhpbHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::THRUSTERS_BOARD) {
				hbStats.thHB.hb = true;
				hbStats.thHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::POPB) {
				hbStats.popbHB.hb = true;
				hbStats.popbHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::ESC1) {
				hbStats.esc1HB.hb = true;
				hbStats.esc1HB.lastReceived = millis();
			}
			else if (device == heartbeat_id::ESC2) {
				hbStats.esc2HB.hb = true;
				hbStats.esc2HB.lastReceived = millis();
			}
			else if (device == heartbeat_id::BATT1) {
				hbStats.batt1HB.hb = true;
				hbStats.batt1HB.lastReceived = millis();
			}
			else if (device == heartbeat_id::BATT2) {
				hbStats.batt2HB.hb = true;
				hbStats.batt2HB.lastReceived = millis();
			}
			else if (device == heartbeat_id::ACTUATION) {
				hbStats.actHB.hb = true;
				hbStats.actHB.lastReceived = millis();
			}
			else if (device == heartbeat_id::NAV_RPI) {
				hbStats.navHB.hb = true;
				hbStats.navHB.lastReceived = millis();
			}
			else {
#ifdef BB_DEBUG
				Serial.println("cannot identify heartbeat device");
#endif
			}
			break;
		}
		case can_id::POWERHULL_STATS: {
			powerHullClimateStats.temp = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			powerHullClimateStats.hum = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			powerHullClimateStats.lastReceived = millis();
			break;
		}
		case can_id::PC_TEMP: {
			pcTempStats.cpuTemp = CAN.parseCANFrame(CAN_msg_buf, 0, 1);
			pcTempStats.gpuTemp = CAN.parseCANFrame(CAN_msg_buf, 1, 1);
			pcTempStats.lastReceived = millis();
			break;
		}
		case can_id::BATT1_STATS: {
			batt1Stats.battVolt = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			batt1Stats.battCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			batt1Stats.battTemp = CAN.parseCANFrame(CAN_msg_buf, 4, 2);
			batt1Stats.lastReceived = millis();
			break;
		}
		case can_id::BATT2_STATS: {
			batt2Stats.battVolt = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			batt2Stats.battCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			batt2Stats.battTemp = CAN.parseCANFrame(CAN_msg_buf, 4, 2);
			batt2Stats.lastReceived = millis();
			break;
		}
		case can_id::MHPB_POWER_STATUS: {
			mhpbStats.mhpbEn = CAN.parseCANFrame(CAN_msg_buf, 0, 1);
			mhpbStats.mhpbCycle = CAN.parseCANFrame(CAN_msg_buf, 1, 1);
			mhpbStats.mhpbFault = CAN.parseCANFrame(CAN_msg_buf, 2, 1);
			mhpbStats.lastReceived = millis();
			break;
		}
		case can_id::KILL_STATUS: {
			uint8_t temp = CAN.parseCANFrame(CAN_msg_buf, 0, 1);
			killStats.hardKill = (temp & kill_bit::HARD) ? RC_Kill::KILLED : RC_Kill::OPERATIONAL;
			killStats.pcKill = (temp & kill_bit::PC) ? RC_Kill::KILLED : RC_Kill::OPERATIONAL;
			killStats.rcKill = (temp & kill_bit::RC) ? RC_Kill::KILLED : RC_Kill::OPERATIONAL;
			killStats.noTelemHBKill = (temp & kill_bit::TELEM_NO_HB) ? RC_Kill::KILLED : RC_Kill::OPERATIONAL;
			killStats.lastReceived = millis();
			break;
		}
		case can_id::ESC1_STATS: {
			portEscStats.bowCurr = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			portEscStats.qtrCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			portEscStats.lastReceived = millis();
			break;
		}
		case can_id::ESC2_STATS: {
			starEscStats.bowCurr = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			starEscStats.qtrCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			starEscStats.lastReceived = millis();
			break;
		}
		case can_id::POPB_PC_STATS: {
			popbPcStats.popbChVolt = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			popbPcStats.popbChCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			popbPcStats.popbChPower = CAN.parseCANFrame(CAN_msg_buf, 4, 2);
			popbPcStats.lastReceived = millis();
			break;
		}
		case can_id::POPB_REST_STATS: {
			popbRestStats.popbChVolt = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			popbRestStats.popbChCurr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			popbRestStats.popbChPower = CAN.parseCANFrame(CAN_msg_buf, 4, 2);
			popbRestStats.lastReceived = millis();
			break;
		}
		case can_id::ACT_STATS: {
			actHullClimateStats.temp = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			actHullClimateStats.hum = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			actHullClimateStats.lastReceived = millis();
			break;
		}
		case can_id::NAV_STATS: {
			navHullClimateStats.temp = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			navHullClimateStats.hum = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			navHullClimateStats.lastReceived = millis();
			break;
		}
		case can_id::ACT_AZI_STATS: {
			thrusterAziStats.portBow = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			thrusterAziStats.portQtr = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			thrusterAziStats.starBow = CAN.parseCANFrame(CAN_msg_buf, 4, 2);
			thrusterAziStats.starQtr = CAN.parseCANFrame(CAN_msg_buf, 6, 2);
			thrusterAziStats.lastReceived = millis();
			break;
		}
		case can_id::ACT_VERT_STATS: {
			vertActStats.portBow = CAN.parseCANFrame(CAN_msg_buf, 0, 1);
			vertActStats.portQtr = CAN.parseCANFrame(CAN_msg_buf, 1, 1);
			vertActStats.starBow = CAN.parseCANFrame(CAN_msg_buf, 2, 1);
			vertActStats.starQtr = CAN.parseCANFrame(CAN_msg_buf, 3, 1);
			vertActStats.hydrophone = CAN.parseCANFrame(CAN_msg_buf, 4, 1);
			vertActStats.lastReceived = millis();
			break;
		}
		case can_id::ACT_BS_STATS: {
			ballshooterStats.pan = CAN.parseCANFrame(CAN_msg_buf, 0, 2);
			ballshooterStats.tilt = CAN.parseCANFrame(CAN_msg_buf, 2, 2);
			ballshooterStats.loader = CAN.parseCANFrame(CAN_msg_buf, 4, 1);
			ballshooterStats.flywheel = CAN.parseCANFrame(CAN_msg_buf, 5, 2);
			ballshooterStats.lastReceived = millis();
			break;
		}

		default:
			/*#ifdef BB_DEBUG
			            Serial.println(CAN.getCanId());
			#endif
			            break;
			        }*/
			break;
		}
		CAN.clearMsg();
	}
}

/**
 * @brief publish heartbeat to CAN
 * @param device_id self (TELEM) device ID
 * @retval None
 *
 */
void publishCANHeartbeat(uint8_t device_id)
{
	CAN_msg_id = can_id::HEARTBEAT;
	CAN_msg_len = 1;
	CAN_msg_buf[0] = device_id;
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish RC kill status to CAN
 * @param killStatus kill status
 * @retval None
 */
void publishKill(RC_Kill killStatus)
{
	CAN_msg_id = can_id::SOFT_KILL;
	CAN_msg_len = 2;
	CAN_msg_buf[0] = static_cast<uint8_t>(killStatus);
	CAN_msg_buf[1] = 2; // 2 - origin from RC, this should be defined in can.hpp instead
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish control mode and RSSI to CAN
 * @param ctrlMode current control mode from RC, auton/RC/station keep
 * @param rssi rssi of rc
 * @retval None
 */
void publishCtrlLink(RC_Vehicle_Mode ctrlMode, int8_t rssi)
{
	CAN_msg_id = can_id::CTRL_LINK;
	CAN_msg_len = 2;
	CAN_msg_buf[0] = static_cast<uint8_t>(ctrlMode);
	CAN_msg_buf[1] = rssi; // publish RSSI (in 1dB)
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish MHPB cycle to CAN
 * @param cycle indicating which channel to cycle
 * @retval None
 */
void publishMHPBCycle(uint8_t cycle)
{
	CAN_msg_id = can_id::RC_MHPB_POWER_CTRL;
	CAN_msg_len = 2;
	CAN_msg_buf[0] = 0x00;
	CAN_msg_buf[1] = cycle;
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish RC control values for thrusters to CAN
 * @param thrusterPortBow port bow - front left
 * @param thrusterPortQtr port quarter - back left
 * @param thrusterStarBow starboard bow - front right
 * @param thrusterStarQtr starboard quarter - back right
 * @retval None
 */
void publishThrusters(uint16_t thrusterPortBow,
                      uint16_t thrusterPortQtr,
                      uint16_t thrusterStarBow,
                      uint16_t thrusterStarQtr)
{
	CAN_msg_id = can_id::RC_THRUSTER;
	CAN_msg_len = 8;
	CAN_msg_buf[6] = thrusterStarQtr;
	CAN_msg_buf[7] = thrusterStarQtr >> 8;
	CAN_msg_buf[4] = thrusterStarBow;
	CAN_msg_buf[5] = thrusterStarBow >> 8;
	CAN_msg_buf[2] = thrusterPortQtr;
	CAN_msg_buf[3] = thrusterPortQtr >> 8;
	CAN_msg_buf[0] = thrusterPortBow;
	CAN_msg_buf[1] = thrusterPortBow >> 8;
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish thrusters and hydrophones deployment to CAN
 * @param thrustDeploy RC_Thrust_Deployment::RETRACTED or RC_Thrust_Deployment::DEPLOYED
 * @param hydroDeploy RC_Hydro_Deployment::RETRACTED or RC_Hydro_Deployment::DEPLOYED
 */
void publishThrustHydroDeploy(RC_Thrust_Deployment thrustDeploy,
                              RC_Hydro_Deployment hydroDeploy,
                              RC_Act_Cal rcActCal)
{
    uint8_t canThrustDeploy = rc_act_vert_ctrl::IGNORE;
    uint8_t canHydroDeploy = rc_act_vert_ctrl::IGNORE;

	// the status for thruster is shared for calibration so call for all four even if one button is pressed only
	if (rcActCal == RC_Act_Cal::PB_VERT || rcActCal == RC_Act_Cal::PQ_VERT || rcActCal == RC_Act_Cal::SB_VERT ||
	    rcActCal == RC_Act_Cal::SQ_VERT)
	{
		canThrustDeploy = rc_act_vert_ctrl::CALIBRATE;
	} else {
        canThrustDeploy = static_cast<uint8_t>(thrustDeploy);
    }

    if (rcActCal == RC_Act_Cal::HY_VERT) {
        canHydroDeploy = rc_act_vert_ctrl::CALIBRATE;
    } else {
        canHydroDeploy = static_cast<uint8_t>(hydroDeploy);
    }

	CAN_msg_id = can_id::RC_ACTUATION_VERT;
	CAN_msg_len = 5;
	CAN_msg_buf[0] = canThrustDeploy;
	CAN_msg_buf[1] = canThrustDeploy;
	CAN_msg_buf[2] = canThrustDeploy;
	CAN_msg_buf[3] = canThrustDeploy;
	CAN_msg_buf[4] = canHydroDeploy;
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish thruster azimuth to CAN based on the two thruster configurations
 * @param thrusterAziMode RC_Thruster_Azi::STATION_KEEP or RC_Thruster_Azi::UNDER_WAY
 */
void publishThrustAziMode(RC_Thruster_Azi thrusterAziMode, RC_Act_Cal rcActCal)
{
	// clang-format off

    /* Station Keeping */
    // arrow points to direction of thrusters positive and direction of force on ASV/opposite of water flow,
    // look from cage side if anti c/w is forward angle is from for aft axis 
    // 1 port bow 45° ↗   ↖ 45° star bow 3
    //                 ASV
    // 2 port qtr 45° ↖   ↗ 45° star qtr 4

    /* Under Way */
    // arrow points to direction of thrusters positive and direction of force on ASV/opposite of water flow,
    // look from cage side if anti c/w is forward angle is from for aft axis
    // 1 port bow 30° ↗   ↖ 30° star bow 3
    //                 ASV
    // 2 port qtr     ↑   ↑     star qtr 4

	// clang-format on

	CAN_msg_id = can_id::RC_THRUSTER_AZI;
	CAN_msg_len = 8;

	uint16_t pbAzi = rc_thruster_azi_ctrl::IGNORE;
	uint16_t pqAzi = rc_thruster_azi_ctrl::IGNORE;
	uint16_t sbAzi = rc_thruster_azi_ctrl::IGNORE;
	uint16_t sqAzi = rc_thruster_azi_ctrl::IGNORE;

	// calculate thruster azimuth based on can standard
	/**
	 * @brief maps -90.00° to 90.00° -> uint16_t from rc_thruster_azi_ctrl::RIGHT to rc_thruster_azi_ctrl::LEFT
	 */
	auto angleToCAN = [](float deg) -> uint16_t {
		deg = (deg < -90.0) ? -90.0 : (deg > 90.0) ? 90.0 : deg;

		return static_cast<uint16_t>(
			((deg - -90.0) * (uint16_t(rc_thruster_azi_ctrl::LEFT) - uint16_t(rc_thruster_azi_ctrl::RIGHT)) /
		     (90.0 - -90.0)) +
			5);
	};

	uint16_t POS_45_DEG = angleToCAN(45.0);
	uint16_t NEG_45_DEG = angleToCAN(-45.0);
	uint16_t POS_30_DEG = angleToCAN(30.0);
	uint16_t NEG_30_DEG = angleToCAN(-30.0);
	uint16_t ZERO_DEG = angleToCAN(0.0);

	if (rcActCal == RC_Act_Cal::PB_AZI) {
		pbAzi = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::STATION_KEEP) {
		// Thruster 1 - Port Bow 45° CW from for aft axis
		pbAzi = POS_45_DEG;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::UNDER_WAY) {
		// Thruster 1 - Port Bow 30° CW from for aft axis
		pbAzi = POS_30_DEG;
	}

	if (rcActCal == RC_Act_Cal::PQ_AZI) {
		pqAzi = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::STATION_KEEP) {
		// Thruster 2 - Port Qtr 45° CCW from for aft axis
		pqAzi = NEG_45_DEG;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::UNDER_WAY) {
		// Thruster 2 - Port Qtr 0° from for aft axis
		pqAzi = ZERO_DEG;
	}

	if (rcActCal == RC_Act_Cal::SB_AZI) {
		sbAzi = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::STATION_KEEP) {
		// Thruster 3 - Star Bow 45° CCW from for aft axis
		sbAzi = NEG_45_DEG;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::UNDER_WAY) {
		// Thruster 3 - Star Bow 30° CCW from for aft axis
		sbAzi = NEG_30_DEG;
	}

	if (rcActCal == RC_Act_Cal::SQ_AZI) {
		sqAzi = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::STATION_KEEP) {
		// Thruster 4 - Star Qtr 45° CW from for aft axis
		sqAzi = POS_45_DEG;
	}
	else if (thrusterAziMode == RC_Thruster_Azi::UNDER_WAY) {
		// Thruster 4 - Star Qtr 0° from for aft axis
		sqAzi = ZERO_DEG;
	}

	CAN_msg_buf[0] = static_cast<uint8_t>(pbAzi & 0xFF);
	CAN_msg_buf[1] = static_cast<uint8_t>((pbAzi >> 8) & 0xFF);

	CAN_msg_buf[2] = static_cast<uint8_t>(pqAzi & 0xFF);
	CAN_msg_buf[3] = static_cast<uint8_t>((pqAzi >> 8) & 0xFF);

	CAN_msg_buf[4] = static_cast<uint8_t>(sbAzi & 0xFF);
	CAN_msg_buf[5] = static_cast<uint8_t>((sbAzi >> 8) & 0xFF);

	CAN_msg_buf[6] = static_cast<uint8_t>(sqAzi & 0xFF);
	CAN_msg_buf[7] = static_cast<uint8_t>((sqAzi >> 8) & 0xFF);

	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

/**
 * @brief publish ball shooter commands to CAN
 * @param pan pan position
 * @param tilt tilt position
 * @param flywheel flywheel speed
 * @param loadShoot idle, load or shoot
 */
void publishBallShooter(uint16_t pan,
                        uint16_t tilt,
                        uint16_t flywheel,
                        RC_Load_Shoot loadShoot,
                        RC_Act_Cal rcActCal)
{
	uint16_t canPan = rc_ballshooter_ctrl::PAN_IGNORE;
	uint16_t canTilt = rc_ballshooter_ctrl::TILT_IGNORE;

	if (rcActCal == RC_Act_Cal::PAN) {
		canPan = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else {
		canPan = pan;
	}

	if (rcActCal == RC_Act_Cal::TILT) {
		canTilt = rc_thruster_azi_ctrl::CALIBRATE;
	}
	else {
		canTilt = tilt;
	}

	CAN_msg_id = can_id::RC_BALLSHOOTER;
	CAN_msg_len = 7;
	CAN_msg_buf[0] = canPan & 0xFF;
	CAN_msg_buf[1] = (canPan >> 8) & 0xFF;
	CAN_msg_buf[2] = canTilt & 0xFF;
	CAN_msg_buf[3] = (canTilt >> 8) & 0xFF;
	CAN_msg_buf[4] = static_cast<uint8_t>(loadShoot);
	CAN_msg_buf[5] = flywheel & 0xFF;
	CAN_msg_buf[6] = (flywheel >> 8) & 0xFF;
	CAN.sendMsgBuf(CAN_msg_id, 0, CAN_msg_len, CAN_msg_buf);
}

//==========================================
//
//        SCREEN FUNCTIONS
//
//==========================================
/**
 * @brief initial screen text for page containing all data
 * @param None
 * @retval None
 */
void prepareScreen()
{
	screen.set_cursor(0 + OFFSET, 0);
	screen.write_string("MH Temp:");
	screen.write_string("PH Temp:");
	screen.write_string("Act Temp:");
	screen.write_string("Nav Temp:");
	screen.write_string("Batt1:");
	screen.write_string("Batt2:");
	screen.write_string("PC:");
	screen.write_string("Rest:");
	screen.write_string_no_increment("Ctrl:");
	screen.set_cursor(250 + OFFSET, 280);
	screen.write_string_no_increment("Kill:");
	screen.set_cursor(0 + OFFSET, 315);
	screen.write_string_no_increment("RSSI RC:");
	screen.set_cursor(250 + OFFSET, 315);
	screen.write_string("RSSI OCS:");
	screen.write_string("GPU Temp:");
	screen.set_cursor(0 + OFFSET, 350);
	screen.write_string("PC Temp:");
	screen.write_string("BS Load:");
	screen.write_string("Act:");
	screen.set_cursor(75 + OFFSET, 420);
	screen.write_string_no_increment("T1:");
	screen.set_cursor(195 + OFFSET, 420);
	screen.write_string_no_increment("T2:");
	screen.set_cursor(315 + OFFSET, 420);
	screen.write_string_no_increment("T3:");
	screen.set_cursor(440 + OFFSET, 420);
	screen.write_string_no_increment("T4:");
	screen.set_cursor(565 + OFFSET, 420);
	screen.write_string_no_increment("HYDRO:");

	screen.set_cursor(270 + OFFSET, 0);
	screen.write_string("MH Hum:");
	screen.write_string("PH Hum:");
	screen.write_string("ACT Hum:");
	screen.write_string("NAV Hum:");
	screen.set_cursor(205 + OFFSET, 140);
	screen.write_string("V");
	screen.write_string("V");
	screen.write_string("V");
	screen.write_string("V");
	screen.set_cursor(340 + OFFSET, 140);
	screen.write_string("A");
	screen.write_string("A");
	screen.write_string("A");
	screen.write_string("A");
	screen.set_cursor(470 + OFFSET, 140);
	screen.write_string("C");
	screen.write_string("C");
	screen.write_string("W");
	screen.write_string("W");

	screen.set_cursor(520 + OFFSET, 0);
	screen.write_string("PC OK:");
	screen.write_string("LBKP OK:");
	screen.write_string("MHPB OK:");
	screen.write_string("THRUS OK:");
	screen.write_string("RC OK:");
	screen.write_string("POPB OK:");
	screen.write_string("ESC1 OK:");
	screen.write_string("ESC2 OK:");
	screen.write_string("BATT1 OK:");
	screen.write_string("BATT2 OK:");
	screen.write_string("ACT OK:");
	screen.write_string("NAV OK:");
}

/**
 * @brief update data on screen
 * @param None
 * @retval None
 * screen display (in order):
 * main temp + humidity, power temp + humidity
 * PC temp, RSSI OCS, RSSI RC
 * right side order: batt 1/2 voltage, batt 1/2 current, batt 1/2 temp, kill
 */
void updateScreenData()
{
	// Main Hull Stats
	screen.set_cursor(150 + OFFSET, 0);
	screen.write_value_dp_no_increment(mainHullClimateStats.temp, 1);
	screen.set_cursor(410 + OFFSET, 0);
	screen.write_value_dp_no_increment(mainHullClimateStats.hum, 1);

	// Power Hull Stats
	screen.set_cursor(150 + OFFSET, 35);
	screen.write_value_dp_no_increment(powerHullClimateStats.temp, 1);
	screen.set_cursor(410 + OFFSET, 35);
	screen.write_value_dp_no_increment(powerHullClimateStats.hum, 1);

	// Act Hull Stats
	screen.set_cursor(150 + OFFSET, 70);
	screen.write_value_dp_no_increment(actHullClimateStats.temp, 1);
	screen.set_cursor(410 + OFFSET, 70);
	screen.write_value_dp_no_increment(actHullClimateStats.hum, 1);

	// Nav Hull Stats
	screen.set_cursor(150 + OFFSET, 105);
	screen.write_value_dp_no_increment(navHullClimateStats.temp, 1);
	screen.set_cursor(410 + OFFSET, 105);
	screen.write_value_dp_no_increment(navHullClimateStats.hum, 1);

	// Batt 1 volt, batt 2 volt, pc volt, rest volt
	screen.set_cursor(110 + OFFSET, 140);
	screen.write_value_with_dp(batt1Stats.battVolt, 1);
	screen.write_value_with_dp(batt2Stats.battVolt, 1);
	screen.write_value_with_dp_custom(popbPcStats.popbChVolt, 1, 80, 30);
	screen.write_value_with_dp_custom(popbRestStats.popbChVolt, 1, 80, 30);

	// Batt 1 curr, 2 curr, pc curr, rest curr
	screen.set_cursor(235 + OFFSET, 140);
	screen.write_value_with_dp(batt1Stats.battCurr, 1);
	screen.write_value_with_dp(batt2Stats.battCurr, 1);
	screen.write_value_with_dp_custom(popbPcStats.popbChCurr, 3, 90, 30);
	screen.write_value_with_dp_custom(popbRestStats.popbChCurr, 3, 90, 30);

	// Batt 1 temp, 2 temp, pc power, rest power
	screen.set_cursor(370 + OFFSET, 140);
	screen.write_value_with_dp(batt1Stats.battTemp, 1);
	screen.write_value_with_dp(batt2Stats.battTemp, 1);
	screen.write_value_with_dp_custom(popbPcStats.popbChPower, 3, 90, 30);
	screen.write_value_with_dp_custom(popbRestStats.popbChPower, 3, 90, 30);
	screen.set_cursor(120 + OFFSET, 280);

	// Control mode
	switch (rcVehMode) {
	case RC_Vehicle_Mode::AUTONOMOUS:
		screen.clear_line();
		screen.write_value_string_custom("AUTO", 100, 30, RA8875_GREEN, RA8875_BLACK, false);
		break;
	case RC_Vehicle_Mode::RC_CTRL_MODE:
		screen.write_value_string_custom("RC", 100, 30, RA8875_ORANGE, RA8875_BLACK, false);
		break;
	case RC_Vehicle_Mode::STATION_KEEP:
		screen.write_value_string_custom("STATION", 100, 30, RA8875_GREEN, RA8875_BLACK, false);
		break;
	default:
		screen.write_value_string_custom("???", 100, 30, RA8875_BLACK, RA8875_YELLOW, false);
		break;
	}

	// Kill status
	screen.set_cursor(340 + OFFSET, 280);
	if (!killStats.isValid) {
		screen.write_value_string_custom("T.O.", 100, 30, RA8875_RED, RA8875_WHITE, true);
	}
	else if (killStats.hardKill == RC_Kill::KILLED) {
		screen.write_value_string_custom("Hard", 100, 30, RA8875_RED, RA8875_WHITE, true);
	}
	else if (killStats.pcKill == RC_Kill::KILLED) {
		screen.write_value_string_custom("PC", 100, 30, RA8875_RED, RA8875_WHITE, true);
	}
	else if (killStats.rcKill == RC_Kill::KILLED) {
		screen.write_value_string_custom("RC", 100, 30, RA8875_RED, RA8875_WHITE, true);
	}
	else if (killStats.noTelemHBKill == RC_Kill::KILLED) {
		screen.write_value_string_custom("HB", 100, 30, RA8875_RED, RA8875_WHITE, true);
	}
	else {
		screen.write_value_string_custom("OK", 100, 30, RA8875_GREEN, RA8875_BLACK, true);
	}

	// RSSI strength (RC and OCS)
	screen.set_cursor(135 + OFFSET, 315);
	rc_rssi = -(crsf.getRxRSSI());
	screen.write_value_int_no_increment(rc_rssi);
	screen.set_cursor(400 + OFFSET, 315);
	screen.write_value_int(ocs_rssi);

	// GPU temp
	screen.write_value_int_no_increment(pcTempStats.gpuTemp);

	// PC temp
	screen.set_cursor(150 + OFFSET, 350);
	screen.write_value_int_no_increment(pcTempStats.cpuTemp);

	// Ballshooter load
	screen.set_cursor(140 + OFFSET, 385);
	if (!ballshooterStats.isValid) {
		screen.write_value_string_custom("T.O.", 240, 30, RA8875_BLACK, RA8875_YELLOW, false);
	}
	else {
		switch (ballshooterStats.loader) {
		case act_bs_stats::LOADER_UNLOADED:
			screen.write_value_string_custom("UNLOADED", 240, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_bs_stats::LOADER_LOADED:
			screen.write_value_string_custom("LOADED", 240, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_bs_stats::LOADER_ERROR:
			screen.write_value_string_custom("ERROR", 240, 30, RA8875_RED, RA8875_WHITE, false);
			break;

		default:
			uint16_t val = ballshooterStats.loader;
			screen.write_value_int_no_increment(val);
			break;
		}
	}

	// Vertical Actuator

	if (!vertActStats.isValid) {
		screen.set_cursor(125 + OFFSET, 420);
		screen.write_value_string_custom("T.O.", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);

		screen.set_cursor(240 + OFFSET, 420);
		screen.write_value_string_custom("T.O.", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);

		screen.set_cursor(365 + OFFSET, 420);
		screen.write_value_string_custom("T.O.", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);

		screen.set_cursor(490 + OFFSET, 420);
		screen.write_value_string_custom("T.O.", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);

		screen.set_cursor(670 + OFFSET, 420);
		screen.write_value_string_custom("T.O.", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
	}
	else {
		screen.set_cursor(125 + OFFSET, 420);
		switch (vertActStats.portBow) {
		case act_vert_stats::HIGHEST:
			screen.write_value_string_custom("UP", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::LOWEST:
			screen.write_value_string_custom("DOWN", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::ERROR:
			screen.write_value_string_custom("!!!", 65, 30, RA8875_RED, RA8875_WHITE, false); // error
			break;
		default:
			screen.write_value_string_custom("MID", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		}

		screen.set_cursor(240 + OFFSET, 420);
		switch (vertActStats.portQtr) {
		case act_vert_stats::HIGHEST:
			screen.write_value_string_custom("UP", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::LOWEST:
			screen.write_value_string_custom("DOWN", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::ERROR:
			screen.write_value_string_custom("!!!", 65, 30, RA8875_RED, RA8875_WHITE, false); // error
			break;
		default:
			screen.write_value_string_custom("MID", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		}

		screen.set_cursor(365 + OFFSET, 420);
		switch (vertActStats.starBow) {
		case act_vert_stats::HIGHEST:
			screen.write_value_string_custom("UP", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::LOWEST:
			screen.write_value_string_custom("DOWN", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::ERROR:
			screen.write_value_string_custom("!!!", 65, 30, RA8875_RED, RA8875_WHITE, false); // error
			break;
		default:
			screen.write_value_string_custom("MID", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		}

		screen.set_cursor(490 + OFFSET, 420);
		switch (vertActStats.starQtr) {
		case act_vert_stats::HIGHEST:
			screen.write_value_string_custom("UP", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::LOWEST:
			screen.write_value_string_custom("DOWN", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::ERROR:
			screen.write_value_string_custom("!!!", 65, 30, RA8875_RED, RA8875_WHITE, false); // error
			break;
		default:
			screen.write_value_string_custom("MID", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		}

		screen.set_cursor(670 + OFFSET, 420);
		switch (vertActStats.hydrophone) {
		case act_vert_stats::HIGHEST:
			screen.write_value_string_custom("UP", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::LOWEST:
			screen.write_value_string_custom("DOWN", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		case act_vert_stats::ERROR:
			screen.write_value_string_custom("!!!", 65, 30, RA8875_RED, RA8875_WHITE, false); // error
			break;
		default:
			screen.write_value_string_custom("MID", 65, 30, RA8875_BLACK, RA8875_YELLOW, false);
			break;
		}
	}
}

/**
 * @brief update all HB data on screen
 * @param None
 * @retval None
 */
void updateHeartbeat()
{
	screen.set_cursor(665 + OFFSET, 0);

	// Array of pointers to the members of the struct
	heartbeat_t* members[] = { &hbStats.pcHB,    &hbStats.lbkpHB,  &hbStats.mhpbHB, &hbStats.thHB,
		                       &hbStats.rcHB,    &hbStats.popbHB,  &hbStats.esc1HB, &hbStats.esc2HB,
		                       &hbStats.batt1HB, &hbStats.batt2HB, &hbStats.actHB,  &hbStats.navHB };

	// Number of members in the array
	size_t numMembers = sizeof(members) / sizeof(members[0]);

	// Iterate through the members, skipping telemHB
	for (size_t i = 0; i < numMembers; i++) {
		if (members[i]->hb) {
			screen.write_value_string("YES");
		}
		else {
			screen.write_value_string("NO");
		}
	}
}

/**
 * @brief Over-arching function to reset stats on screen (display "T.O." if no data received)
 * @param None
 * @retval None
 */
void resetStats()
{
	resetHeartBeat();
	resetMainHullStats();
	resetPowerHullStats();
	resetBatteryStats();
	resetPOPBStats();
	resetMHPBStats();
	resetSBCStats();
	resetActHullStats();
	resetNavHullStats();
	resetBallShooterStats();
	resetActAziStats();
	resetActVertStats();
}

/**
 * @brief reset heartbeat stats
 * @param None
 * @retval None
 */
void resetHeartBeat()
{
	// Pointer to the first member of the struct
	heartbeat_t* ptr = &hbStats.pcHB;

	// Number of members in the struct
	size_t numMembers = sizeof(hbStats) / sizeof(heartbeat_t);

	// Iterate through the members
	for (size_t i = 0; i < numMembers; i++) {
		ptr[i].hb = (millis() - ptr[i].lastReceived) < HB_TIMEOUT;
	}
}

/**
 * @brief reset main hull stats
 * @param None
 * @retval None
 */
void resetMainHullStats()
{
	mainHullClimateStats.isValid = (millis() - mainHullClimateStats.lastReceived) < STATS_TIMEOUT;
	if (!mainHullClimateStats.isValid) {
		mainHullClimateStats.temp = 0xFFFF;
		mainHullClimateStats.hum = 0xFFFF;
	}
}

/**
 * @brief reset power hull stats
 * @param None
 * @retval None
 */
void resetPowerHullStats()
{
	powerHullClimateStats.isValid = (millis() - powerHullClimateStats.lastReceived) < STATS_TIMEOUT;
	if (!powerHullClimateStats.isValid) {
		powerHullClimateStats.temp = 0xFFFF;
		powerHullClimateStats.hum = 0xFFFF;
	}
}

/**
 * @brief reset battery stats
 * @param None
 * @retval None
 */
void resetBatteryStats()
{
	batt1Stats.isValid = (millis() - batt1Stats.lastReceived) < STATS_TIMEOUT;
	batt2Stats.isValid = (millis() - batt2Stats.lastReceived) < STATS_TIMEOUT;
	if (!batt1Stats.isValid) {
		batt1Stats.battVolt = 0xFFFF;
		batt1Stats.battCurr = 0xFFFF;
		batt1Stats.battTemp = 0xFFFF;
	}
	if (!batt2Stats.isValid) {
		batt2Stats.battVolt = 0xFFFF;
		batt2Stats.battCurr = 0xFFFF;
		batt2Stats.battTemp = 0xFFFF;
	}
}

/**
 * @brief reset popb stats (PC + REST STATS)
 * @param None
 * @retval None
 */
void resetPOPBStats()
{
	popbPcStats.isValid = (millis() - popbPcStats.lastReceived) < STATS_TIMEOUT;
	popbRestStats.isValid = (millis() - popbRestStats.lastReceived) < STATS_TIMEOUT;
	if (!popbPcStats.isValid) {
		popbPcStats.popbChVolt = 0xFFFF;
		popbPcStats.popbChCurr = 0xFFFF;
		popbPcStats.popbChPower = 0xFFFF;
	}
	if (!popbRestStats.isValid) {
		popbRestStats.popbChVolt = 0xFFFF;
		popbRestStats.popbChCurr = 0xFFFF;
		popbRestStats.popbChPower = 0xFFFF;
	}
}

/**
 * @brief reset mhpb stats
 * @param None
 * @retval None
 */
void resetMHPBStats()
{
	mhpbStats.isValid = (millis() - mhpbStats.lastReceived) < STATS_TIMEOUT;
}

/**
 * @brief reset PC temp stats
 * @param None
 * @retval None
 */
void resetSBCStats()
{
	pcTempStats.isValid = (millis() - pcTempStats.lastReceived) < STATS_TIMEOUT;
	if (!pcTempStats.isValid) {
		pcTempStats.cpuTemp = 0xFFFF;
		pcTempStats.gpuTemp = 0xFFFF;
	}
}

/**
 * @brief reset Actuation Hull stats
 * @param None
 * @retval None
 */
void resetActHullStats()
{
	actHullClimateStats.isValid = (millis() - actHullClimateStats.lastReceived) < STATS_TIMEOUT;
	if (!actHullClimateStats.isValid) {
		actHullClimateStats.temp = 0xFFFF;
		actHullClimateStats.hum = 0xFFFF;
	}
}

/**
 * @brief reset Navigation Hull stats
 * @param None
 * @retval None
 */
void resetNavHullStats()
{
	navHullClimateStats.isValid = (millis() - navHullClimateStats.lastReceived) < STATS_TIMEOUT;
	if (!navHullClimateStats.isValid) {
		navHullClimateStats.temp = 0xFFFF;
		navHullClimateStats.hum = 0xFFFF;
	}
}

/**
 * @brief reset Ballshooter stats
 * @param None
 * @retval None
 */
void resetBallShooterStats()
{
	ballshooterStats.isValid = (millis() - ballshooterStats.lastReceived) < STATS_TIMEOUT;
	if (!ballshooterStats.isValid) {
		ballshooterStats.pan = 0xFFFF;
		ballshooterStats.tilt = 0xFFFF;
		ballshooterStats.flywheel = 0xFFFF;
	}
}

/**
 * @brief reset actuator azi stats
 * @param None
 * @retval None
 */
void resetActAziStats()
{
	thrusterAziStats.isValid = (millis() - thrusterAziStats.lastReceived) < STATS_TIMEOUT;
}

/**
 * @brief reset actuator vert stats
 * @param None
 * @retval None
 */
void resetActVertStats()
{
	vertActStats.isValid = (millis() - vertActStats.lastReceived) < STATS_TIMEOUT;
}

//==========================================
//
//        RC FUNCTIONS
//
//==========================================

/**
 * @brief calls for read packet when there is serial data.
 *        serialEvent3 is ran after each loop when data is available, its not an ISR
 * @param None
 * @retval None
 */
void serialEvent3()
{
	// only read when buffer is half full so will not read too fast and discards packets
	if (crsf.serialBytesAvailable() > 32) {
		digitalWrite(GENERAL_LED1, HIGH);
		crsf.readPacket();
		digitalWrite(GENERAL_LED1, LOW);
	}
}

/**
 * @brief control RSSI leds based on RSSI input, blink all 4 leds if controller not connected
 * @param rssi
 * @param isTxConnected
 * @retval None
 */
void rssiLeds(int8_t rssi, bool isTxConnected)
{
	if (isTxConnected) {
		// controller connected, led based on RSSI level
		if (rssi > RSSI_BEST) {
			digitalWrite(RSSI_LED1, HIGH);
			digitalWrite(RSSI_LED2, HIGH);
			digitalWrite(RSSI_LED3, HIGH);
			digitalWrite(RSSI_LED4, HIGH);
		}
		else if (rssi > RSSI_GOOD) {
			digitalWrite(RSSI_LED1, LOW);
			digitalWrite(RSSI_LED2, HIGH);
			digitalWrite(RSSI_LED3, HIGH);
			digitalWrite(RSSI_LED4, HIGH);
		}
		else if (rssi > RSSI_OKAY) {
			digitalWrite(RSSI_LED1, LOW);
			digitalWrite(RSSI_LED2, LOW);
			digitalWrite(RSSI_LED3, HIGH);
			digitalWrite(RSSI_LED4, HIGH);
		}
		else if (rssi > RSSI_NOTGOOD) {
			digitalWrite(RSSI_LED1, LOW);
			digitalWrite(RSSI_LED2, LOW);
			digitalWrite(RSSI_LED3, LOW);
			digitalWrite(RSSI_LED4, HIGH);
		}
		else {
			digitalWrite(RSSI_LED1, LOW);
			digitalWrite(RSSI_LED2, LOW);
			digitalWrite(RSSI_LED3, LOW);
			digitalWrite(RSSI_LED4, LOW);
		}
	}
	else {
		// controller disconnected, blink the leds
		static unsigned long last_millis = 0;
		if (millis() - last_millis <= 500) {
			digitalWrite(RSSI_LED1, HIGH);
			digitalWrite(RSSI_LED2, HIGH);
			digitalWrite(RSSI_LED3, HIGH);
			digitalWrite(RSSI_LED4, HIGH);
		}
		else if (millis() - last_millis <= 1000) {
			digitalWrite(RSSI_LED1, LOW);
			digitalWrite(RSSI_LED2, LOW);
			digitalWrite(RSSI_LED3, LOW);
			digitalWrite(RSSI_LED4, LOW);
		}
		else {
			last_millis = millis();
		}
	}
}

void hackyWhackyTelemetry(heartbeat_stats_t hbStats,
                          battery_stats_t batt1Stats,
                          battery_stats_t batt2Stats,
                          popb_stats_t popbPcStats,
                          popb_stats_t popbRestStats,
                          mhpb_stats_t mhpbStats,
                          hull_climate_stats_t mainHullClimateStats,
                          hull_climate_stats_t powerHullClimateStats,
                          hull_climate_stats_t actHullClimateStats,
                          hull_climate_stats_t navHullClimateStats,
                          pc_temp_t pcTempStats,
                          thruster_azi_stats_t thrusterAziStats,
                          vert_act_stats_t vertActStats,
                          ballshooter_stats_t ballshooterStats,
                          kill_status_t killStats,
                          RC_Vehicle_Mode rcVehMode)
{
	// helper function to convert actuation stats to sendable values

	/**
	 * @brief maps uint16_t from 5 to 65535 -> -90.00° to 90.00°
	 *        error and status to value specified in hackyWhackyTelemetry sheet
	 */
	auto canPanTiltToAngle = [](uint16_t canVal) -> float {
		// TODO: case number should be from CAN standard
		switch (canVal) {
		case 0:
			return RC_TELEM_TILTPAN_ERROR;
			break;
		case 1:
			return RC_TELEM_TILTPAN_CALIBRATING;
			break;
		case 2:
			return RC_TELEM_TILTPAN_STATUS2;
			break;
		case 3:
			return RC_TELEM_TILTPAN_STATUS3;
			break;
		case 4:
			return RC_TELEM_TILTPAN_STATUS4;
			break;
		default:
			return (float(canVal - 5) * (90.0 - -90.0) / (65535.0 - 5.0)) - 90.0;
			break;
		}
	};

	/**
	 * @brief maps uint16_t from 5 to 65535 -> 1 to 181 (TX16s will map to -90° to 90°)
	 *        error and status to value specified in hackyWhackyTelemetry sheet
	 */
	auto canAziToAngle = [](uint16_t canVal) -> uint8_t {
		// TODO: case number should be from CAN standard
		switch (canVal) {
		case 0:
			return RC_TELEM_AZI_ERROR;
			break;
		case 1:
			return RC_TELEM_AZI_CALIBRATING;
			break;
		case 2:
			return RC_TELEM_AZI_STATUS2;
			break;
		case 3:
			return RC_TELEM_AZI_STATUS3;
			break;
		case 4:
			return RC_TELEM_AZI_STATUS4;
			break;
		default:
			return uint8_t((canVal - 5.0) * (180.0 - 0) / (65535.0 - 5.0)) + 1;
			break;
		}
	};

	/**
	 * @brief maps uint16_t from 5 to 65535 -> 1 to 201 (TX16s will map to -100% to 100%)
	 *        error and status to value specified in hackyWhackyTelemetry sheet
	 */
	auto canFlywheelToPercent = [](uint16_t canVal) -> uint8_t {
		// TODO: case number should be from CAN standard
		switch (canVal) {
		case 0:
			return RC_TELEM_FLYWHEEL_ERROR;
			break;
		case 1:
			return RC_TELEM_FLYWHEEL_STATUS1;
			break;
		case 2:
			return RC_TELEM_FLYWHEEL_STATUS2;
			break;
		case 3:
			return RC_TELEM_FLYWHEEL_STATUS3;
			break;
		case 4:
			return RC_TELEM_FLYWHEEL_STATUS4;
			break;
		default:
			return uint8_t((canVal - 5.0) * (200.0 - 0) / (65535.0 - 5.0)) + 1;
			break;
		}
	};

	/**
	 * @brief convert vertical actuator status to tri-state coefficient {RETRACTED, DEPLOYED, MOVING/ERROR}
	 */
	auto canVertToTriState = [](uint8_t canVal) -> RC_Telem_Vert_Act {
		// TODO: case number should be from CAN standard
		switch (canVal) {
		case act_vert_stats::ERROR:
			return RC_Telem_Vert_Act::ERROR;
			break;
		case 1:
			return RC_Telem_Vert_Act::CALIBRATING;
			break;
		case 2:
			// treat status 2 as error, no more bits
			return RC_Telem_Vert_Act::ERROR;
			break;
		case 3:
			// treat status 3 as error, no more bits
			return RC_Telem_Vert_Act::ERROR;
			break;
		case 4:
			// treat status 4 as error, no more bits
			return RC_Telem_Vert_Act::ERROR;
			break;
		case 5:
			return RC_Telem_Vert_Act::DEPLOYED;
			break;
		case act_vert_stats::HIGHEST:
			return RC_Telem_Vert_Act::RETRACTED;
			break;
		default:
			return RC_Telem_Vert_Act::MOVING;
			break;
		}
	};

	/**
	 * @brief convert vertical actuators status to MOD-3 value
	 */
	auto vertTriStatesToByte = [canVertToTriState](vert_act_stats_t vertActStats) -> uint8_t {
		RC_Telem_Vert_Act pbStats = canVertToTriState(vertActStats.portBow);
		RC_Telem_Vert_Act pqStats = canVertToTriState(vertActStats.portQtr);
		RC_Telem_Vert_Act sbStats = canVertToTriState(vertActStats.starBow);
		RC_Telem_Vert_Act sqStats = canVertToTriState(vertActStats.starQtr);
		RC_Telem_Vert_Act hyStats = canVertToTriState(vertActStats.hydrophone);

		// if there are any error will send 255 - MOD-2
		// this means if there is any error will only show error and not other status
		// do not have enough bits, port side and starboard side thrusters actuation error are combined
		if (pbStats == RC_Telem_Vert_Act::ERROR || pqStats == RC_Telem_Vert_Act::ERROR ||
		    sbStats == RC_Telem_Vert_Act::ERROR || sqStats == RC_Telem_Vert_Act::ERROR ||
		    hyStats == RC_Telem_Vert_Act::ERROR)
		{
			uint8_t temp = 255;

			if (pbStats == RC_Telem_Vert_Act::ERROR || pqStats == RC_Telem_Vert_Act::ERROR) {
				temp -= 4;
			}

			if (sbStats == RC_Telem_Vert_Act::ERROR || sqStats == RC_Telem_Vert_Act::ERROR) {
				temp -= 2;
			}

			if (hyStats == RC_Telem_Vert_Act::ERROR) {
				temp -= 1;
			}

			return temp;
		}

		// if there are no errors but there is calibrating status, will send 248 - MOD-2
		// this means if there is any calibrating status will only show calibrating and not other status
		// do not have enough bits, thrusters actuation calibrating are combined
		if (pbStats == RC_Telem_Vert_Act::CALIBRATING || pqStats == RC_Telem_Vert_Act::CALIBRATING ||
		    sbStats == RC_Telem_Vert_Act::CALIBRATING || sqStats == RC_Telem_Vert_Act::CALIBRATING ||
		    hyStats == RC_Telem_Vert_Act::CALIBRATING)
		{
			uint8_t temp = 248;

			if (pbStats == RC_Telem_Vert_Act::CALIBRATING || pqStats == RC_Telem_Vert_Act::CALIBRATING ||
			    sbStats == RC_Telem_Vert_Act::CALIBRATING || sqStats == RC_Telem_Vert_Act::CALIBRATING)
			{
				temp -= 2;
			}

			if (hyStats == RC_Telem_Vert_Act::CALIBRATING) {
				temp -= 1;
			}

			return temp;
		}

		// if there are no errors and no calibrating status, will send MOD-3 value
		// deployed, moving, retracted for each thruster and hydrophone
		uint8_t pbCoeff = uint8_t(pbStats);
		uint8_t pqCoeff = uint8_t(pqStats);
		uint8_t sbCoeff = uint8_t(sbStats);
		uint8_t sqCoeff = uint8_t(sqStats);
		uint8_t hyCoeff = uint8_t(hyStats);

		return hyCoeff * 81 + sqCoeff * 27 + sbCoeff * 9 + pqCoeff * 3 + pbCoeff;
	};

	// struct and array to memcpy from
	typedef struct
	{
		// GPS frame
		long tilt : 32;

		long pan : 32;

		unsigned int : 1;
		unsigned int cpuTemp : 7;

		unsigned int mhpbEn : 8;

		unsigned int : 1;
		unsigned int actHullHum : 7;

		unsigned int mhpbCycle : 8;

		unsigned int : 1;
		unsigned int batt2Temp : 7;

		unsigned int mhpbFault : 8;

		unsigned int : 1;
		unsigned int gpuTemp : 7;

		// Battery frame
		unsigned int : 1;
		unsigned int batt1Temp : 7;

		unsigned int killStats : 2;
		unsigned int ctrlMode : 2;
		unsigned int loader : 2;
		unsigned int mhpbHB : 1;
		unsigned int thHB : 1;

		unsigned int : 1;
		unsigned int navHullTemp : 7;

		unsigned int vertAct : 8;

		unsigned int : 1;
		unsigned int batt1Volt : 9;
		unsigned int batt2Volt : 9;
		unsigned int rcHB : 1;
		unsigned int esc1HB : 1;
		unsigned int esc2HB : 1;
		unsigned int batt2HB : 1;
		unsigned int navHB : 1;

		unsigned int : 1;
		unsigned int navHullHum : 7;

		// Attitude frame
		unsigned int popbPcCurr : 16;

		unsigned int batt1Curr : 16;

		unsigned int batt2Curr : 16;

		// FLight mode frame
		unsigned int popbRestCurr : 8;
		unsigned int popbPcVolt : 8;
		unsigned int popbRestVolt : 8;
		unsigned int portBowAzi : 8;
		unsigned int portQtrAzi : 8;
		unsigned int starBowAzi : 8;
		unsigned int starQtrAzi : 8;
		unsigned int flyweel : 8;

		unsigned int mainHullTemp : 7;
		unsigned int pcHb : 1;

		unsigned int mainHullHum : 7;
		unsigned int lbkpHB : 1;

		unsigned int powerHullTemp : 7;
		unsigned int popbHB : 1;

		unsigned int powerHullHum : 7;
		unsigned int batt1HB : 1;

		unsigned int actHullTemp : 7;
		unsigned int actHB : 1;
	} __attribute__((__packed__)) asv_telem_t;

	// initialise all to 0, this ensures the anonymous padding bits are also 0
	asv_telem_t asvTelem = {};

	// struct and array to memcpy into
	typedef struct
	{
		int32_t latitude;
		int32_t longitude;
		uint16_t groundSpeed;
		uint16_t gpsHeading;
		uint16_t altitude;
		uint8_t satUsed;
	} __attribute__((__packed__)) crsf_gps_t;

	typedef struct
	{
		uint16_t batt_voltage;
		uint16_t batt_current;
		uint32_t fuel : 24;
		uint8_t battPercent;
	} __attribute__((__packed__)) crsf_battery_t;

	typedef struct
	{
		uint16_t pitch;
		uint16_t roll;
		uint16_t yaw;
	} __attribute__((__packed__)) crsf_attitude_t;

	typedef struct
	{
		crsf_gps_t crsf_gps;
		crsf_battery_t crsf_batt;
		crsf_attitude_t crsf_attitude;
		char flightMode[13];
	} __attribute__((__packed__)) crsf_telem_t;

	crsf_telem_t crsfTelem;

	/*
	    Below fields doesnt need to check for overflow
	*/
	// mhpb doesnt have enough bits for CAN TIMEOUT, but send the impossible combination of all off, Cycling and
	// Fault
	if (mhpbStats.isValid) {
		asvTelem.mhpbEn = mhpbStats.mhpbEn;
		asvTelem.mhpbCycle = mhpbStats.mhpbCycle;
		asvTelem.mhpbFault = mhpbStats.mhpbFault;
	}
	else {
		asvTelem.mhpbEn = 0xFF;
		asvTelem.mhpbCycle = 0xFF;
		asvTelem.mhpbFault = 0xFF;
	}

	// heart beat itself is a timeout
	asvTelem.pcHb = hbStats.pcHB.hb;
	asvTelem.lbkpHB = hbStats.lbkpHB.hb;
	asvTelem.mhpbHB = hbStats.mhpbHB.hb;
	asvTelem.thHB = hbStats.thHB.hb;
	asvTelem.rcHB = hbStats.rcHB.hb;
	asvTelem.popbHB = hbStats.popbHB.hb;
	asvTelem.esc1HB = hbStats.esc1HB.hb;
	asvTelem.esc2HB = hbStats.esc2HB.hb;
	asvTelem.batt1HB = hbStats.batt1HB.hb;
	asvTelem.batt2HB = hbStats.batt2HB.hb;
	asvTelem.actHB = hbStats.actHB.hb;
	asvTelem.navHB = hbStats.navHB.hb;

	// killstats doesnt have enough bits for CAN TIMEOUT
	// RC_Telem_Kill has well defined values so can int()
	if (killStats.hardKill == RC_Kill::KILLED) {
		asvTelem.killStats = int(RC_Telem_Kill::HARD_KILLED);
	}
	else if (killStats.pcKill == RC_Kill::KILLED) {
		asvTelem.killStats = int(RC_Telem_Kill::PC_KILLED);
	}
	else if (killStats.rcKill == RC_Kill::KILLED) {
		asvTelem.killStats = int(RC_Telem_Kill::RC_KILLED);
	}
	else if (killStats.noTelemHBKill == RC_Kill::KILLED) {
		asvTelem.killStats = int(RC_Telem_Kill::NO_TELEM_HB_KILLED);
	}
	else {
		asvTelem.killStats = int(RC_Telem_Kill::NONE);
	}

	// control mode is internal has no CAN TIMEOUT
	// RC_Vehicle_Mode has well defined values so can int()
	asvTelem.ctrlMode = int(rcVehMode);

	/*
	    Clean up value to avoid overflow, set to 0 if overflow
	    Otherwise do formatting required to send to Rx
	*/

	// battVolt is in dV, no need conversion, only need to take care of overflow
	// battCurr is in dA, need to x10 and to take care of overflow
	// battTemp is in d°C need /10 to °C and need to take care of overflow
	if (batt1Stats.isValid) {
		asvTelem.batt1Volt = (batt1Stats.battVolt > RC_TELEM_BATT_V_MAX) ? RC_TELEM_BATT_V_MAX
		                                                                 : batt1Stats.battVolt;

		asvTelem.batt1Curr = (uint16_t(batt1Stats.battCurr) * 10 > RC_TELEM_BATT_I_MAX)
		                         ? RC_TELEM_BATT_I_MAX
		                         : uint16_t(batt1Stats.battCurr) * 10;

		asvTelem.batt1Temp = (uint16_t(batt1Stats.battTemp / 10) > RC_TELEM_TEMP_MAX)
		                         ? RC_TELEM_TEMP_MAX
		                         : (uint8_t(batt1Stats.battTemp / 10)) + 1;
	}
	else {
		asvTelem.batt1Volt = RC_TELEM_BATT_V_TIMEOUT;
		asvTelem.batt1Curr = RC_TELEM_BATT_I_TIMEOUT;
		asvTelem.batt1Temp = RC_TELEM_TEMP_TIMEOUT;
	}

	if (batt2Stats.isValid) {
		asvTelem.batt2Volt = (batt2Stats.battVolt > RC_TELEM_BATT_V_MAX) ? RC_TELEM_BATT_V_MAX
		                                                                 : batt2Stats.battVolt;

		asvTelem.batt2Curr = (uint16_t(batt2Stats.battCurr) * 10 > RC_TELEM_BATT_I_MAX)
		                         ? RC_TELEM_BATT_I_MAX
		                         : uint16_t(batt2Stats.battCurr) * 10;

		asvTelem.batt2Temp = (uint16_t(batt2Stats.battTemp / 10) > RC_TELEM_TEMP_MAX)
		                         ? RC_TELEM_TEMP_MAX
		                         : (uint8_t(batt2Stats.battTemp / 10)) + 1;
	}
	else {
		asvTelem.batt2Volt = RC_TELEM_BATT_V_TIMEOUT;
		asvTelem.batt2Curr = RC_TELEM_BATT_I_TIMEOUT;
		asvTelem.batt2Temp = RC_TELEM_TEMP_TIMEOUT;
	}

	// popbPcStats.popbChVolt is in dV, only need to take care of overflow
	// popbPcStats.popbChCurr is in mA, need to / 10 to cA and need to take care of overflow
	if (popbPcStats.isValid) {
		asvTelem.popbPcVolt = (popbPcStats.popbChVolt > RC_TELEM_PC_V_MAX) ? RC_TELEM_PC_V_MAX
		                                                                   : popbPcStats.popbChVolt + 1;

		asvTelem.popbPcCurr = (popbPcStats.popbChCurr / 10 > RC_TELEM_PC_I_MAX) ? RC_TELEM_PC_I_MAX
		                                                                        : popbPcStats.popbChCurr / 10;
	}
	else {
		asvTelem.popbPcVolt = RC_TELEM_PC_V_TIMEOUT;
		asvTelem.popbPcCurr = RC_TELEM_PC_I_TIMEOUT;
	}

	// popbRestStats.popbChVolt is in dV, only need to take care of overflow
	// popbRestStats.popbChCurr is in mA, need to / 100 to dA and need to take care of overflow
	if (popbRestStats.isValid) {
		asvTelem.popbRestVolt = (popbRestStats.popbChVolt > RC_TELEM_REST_V_MAX) ? RC_TELEM_REST_V_MAX
		                                                                         : popbRestStats.popbChVolt + 1;

		asvTelem.popbRestCurr = (popbRestStats.popbChCurr / 100 > RC_TELEM_REST_I_MAX)
		                            ? RC_TELEM_REST_I_MAX
		                            : (popbRestStats.popbChCurr / 100) + 1;
	}
	else {
		asvTelem.popbRestVolt = RC_TELEM_REST_V_TIMEOUT;
		asvTelem.popbRestCurr = RC_TELEM_REST_I_TIMEOUT;
	}

	// mainHullTemp is in d°C need /10 to °C and need to take care of overflow
	// mainHullHum are all in d%, need /10 to % and need to take care of overflow
	if (mainHullClimateStats.isValid) {
		asvTelem.mainHullTemp = (uint16_t(mainHullClimateStats.temp / 10) > RC_TELEM_TEMP_MAX)
		                            ? RC_TELEM_TEMP_MAX
		                            : uint8_t(mainHullClimateStats.temp / 10) + 1;

		asvTelem.mainHullHum = (uint16_t(mainHullClimateStats.hum / 10) > RC_TELEM_HUM_MAX)
		                           ? RC_TELEM_HUM_MAX
		                           : uint8_t(mainHullClimateStats.hum / 10) + 1;
	}
	else {
		asvTelem.mainHullTemp = RC_TELEM_TEMP_TIMEOUT;
		asvTelem.mainHullHum = RC_TELEM_HUM_TIMEOUT;
	}

	// powerHullTemp is in d°C need /10 to °C and need to take care of overflow
	// powerHullHum are all in d%, need /10 to % and need to take care of overflow
	if (powerHullClimateStats.isValid) {
		asvTelem.powerHullTemp = (uint16_t(powerHullClimateStats.temp / 10) > RC_TELEM_TEMP_MAX)
		                             ? RC_TELEM_TEMP_MAX
		                             : uint8_t(powerHullClimateStats.temp / 10) + 1;

		asvTelem.powerHullHum = (uint16_t(powerHullClimateStats.hum / 10) > RC_TELEM_HUM_MAX)
		                            ? RC_TELEM_HUM_MAX
		                            : uint8_t(powerHullClimateStats.hum / 10) + 1;
	}
	else {
		asvTelem.powerHullTemp = RC_TELEM_TEMP_TIMEOUT;
		asvTelem.powerHullHum = RC_TELEM_HUM_TIMEOUT;
	}

	// actHullTemp is in d°C need /10 to °C and need to take care of overflow
	// actHullHum are all in d%, need /10 to % and need to take care of overflow
	if (actHullClimateStats.isValid) {
		asvTelem.actHullTemp = (uint16_t(actHullClimateStats.temp / 10) > RC_TELEM_TEMP_MAX)
		                           ? RC_TELEM_TEMP_MAX
		                           : uint8_t(actHullClimateStats.temp / 10) + 1;

		asvTelem.actHullHum = (uint16_t(actHullClimateStats.hum / 10) > RC_TELEM_HUM_MAX)
		                          ? RC_TELEM_HUM_MAX
		                          : uint8_t(actHullClimateStats.hum / 10) + 1;
	}
	else {
		asvTelem.actHullTemp = RC_TELEM_TEMP_TIMEOUT;
		asvTelem.actHullHum = RC_TELEM_HUM_TIMEOUT;
	}

	// navHullTemp is in d°C need /10 to °C and need to take care of overflow
	// navHullHum are all in d%, need /10 to % and need to take care of overflow
	if (navHullClimateStats.isValid) {
		asvTelem.navHullTemp = (uint16_t(navHullClimateStats.temp / 10) > RC_TELEM_TEMP_MAX)
		                           ? RC_TELEM_TEMP_MAX
		                           : uint8_t(navHullClimateStats.temp / 10) + 1;

		asvTelem.navHullHum = (uint16_t(navHullClimateStats.hum / 10) > RC_TELEM_HUM_MAX)
		                          ? RC_TELEM_HUM_MAX
		                          : uint8_t(navHullClimateStats.hum / 10) + 1;
	}
	else {
		asvTelem.navHullTemp = RC_TELEM_TEMP_TIMEOUT;
		asvTelem.navHullHum = RC_TELEM_HUM_TIMEOUT;
	}

	// cpuTemp is in °C, no need conversion, need to take care of overflow
	// gpuTemp is in °C, no need conversion, need to take care of overflow
	if (pcTempStats.isValid) {
		asvTelem.cpuTemp = (uint16_t(pcTempStats.cpuTemp) > RC_TELEM_TEMP_MAX) ? RC_TELEM_TEMP_MAX
		                                                                       : uint8_t(pcTempStats.cpuTemp) + 1;

		asvTelem.gpuTemp = (uint16_t(pcTempStats.gpuTemp) > RC_TELEM_TEMP_MAX) ? RC_TELEM_TEMP_MAX
		                                                                       : uint8_t(pcTempStats.gpuTemp) + 1;
	}
	else {
		asvTelem.cpuTemp = RC_TELEM_TEMP_TIMEOUT;
		asvTelem.gpuTemp = RC_TELEM_TEMP_TIMEOUT;
	}

	// tilt and pan are in -90° to 90° and need to * 10_000_000 to send to Rx

	if (ballshooterStats.isValid) {
		asvTelem.flyweel = canFlywheelToPercent(ballshooterStats.flywheel);

		asvTelem.tilt = int32_t(canPanTiltToAngle(ballshooterStats.tilt) * 10000000);
		asvTelem.pan = int32_t(canPanTiltToAngle(ballshooterStats.pan) * 10000000);

		asvTelem.loader = ballshooterStats.loader;
	}
	else {
		asvTelem.flyweel = RC_TELEM_FLYWHEEL_TIMEOUT;

		asvTelem.tilt = RC_TELEM_TILTPAN_TIMEOUT * 10000000;
		asvTelem.pan = RC_TELEM_TILTPAN_TIMEOUT * 10000000;

		asvTelem.loader = RC_TELEM_LOADER_TIMEOUT;
	}

	// vertical actuators is mapped to MOD-3 value
	// errors will be 255 - MOD-2 value
	// calibrating will be 248 - MOD-2 value
	if (vertActStats.isValid) {
		asvTelem.vertAct = vertTriStatesToByte(vertActStats);
	}
	else {
		asvTelem.vertAct = RC_TELEM_VERT_ACT_TIMEOUT;
	}

	// azimuth
	if (thrusterAziStats.isValid) {
		asvTelem.portBowAzi = canAziToAngle(thrusterAziStats.portBow);
		asvTelem.portQtrAzi = canAziToAngle(thrusterAziStats.portQtr);
		asvTelem.starBowAzi = canAziToAngle(thrusterAziStats.starBow);
		asvTelem.starQtrAzi = canAziToAngle(thrusterAziStats.starQtr);
	}
	else {
		asvTelem.portBowAzi = RC_TELEM_AZI_TIMEOUT;
		asvTelem.portQtrAzi = RC_TELEM_AZI_TIMEOUT;
		asvTelem.starBowAzi = RC_TELEM_AZI_TIMEOUT;
		asvTelem.starQtrAzi = RC_TELEM_AZI_TIMEOUT;
	}

	// Copy data from telemetry struct to ASV telemetry struct
	// This transfers the bytes over without the need of shifting and shits
	memcpy(&crsfTelem, &asvTelem, RC_TELEM_TOTAL_BYTES);

	// Send data to Rx
	crsf.writeGPSPacket(crsfTelem.crsf_gps.latitude,
	                    crsfTelem.crsf_gps.longitude,
	                    crsfTelem.crsf_gps.groundSpeed,
	                    crsfTelem.crsf_gps.gpsHeading,
	                    crsfTelem.crsf_gps.altitude,
	                    crsfTelem.crsf_gps.satUsed);

	crsf.writeBatteryPacket(crsfTelem.crsf_batt.batt_voltage,
	                        crsfTelem.crsf_batt.batt_current,
	                        crsfTelem.crsf_batt.fuel,
	                        crsfTelem.crsf_batt.battPercent);

	crsf.writeAttitudePacket(crsfTelem.crsf_attitude.pitch,
	                         crsfTelem.crsf_attitude.roll,
	                         crsfTelem.crsf_attitude.yaw);

	crsf.writeFlightModePacket(crsfTelem.flightMode, sizeof(crsfTelem.flightMode));

	// // debug print
	// Serial.println(" Converting input to sendable forms ========================================");

	// Serial.print("tilt: ");
	// Serial.println(asvTelem.tilt);
	// Serial.print("pan: ");
	// Serial.println(asvTelem.pan);
	// Serial.print("cpuTemp: ");
	// Serial.println(asvTelem.cpuTemp);
	// Serial.print("mhpbEn: ");
	// Serial.println(asvTelem.mhpbEn);
	// Serial.print("actHullHum: ");
	// Serial.println(asvTelem.actHullHum);
	// Serial.print("mhpbCycle: ");
	// Serial.println(asvTelem.mhpbCycle);
	// Serial.print("batt2Temp: ");
	// Serial.println(asvTelem.batt2Temp);
	// Serial.print("mhpbFault: ");
	// Serial.println(asvTelem.mhpbFault);
	// Serial.print("gpuTemp: ");
	// Serial.println(asvTelem.gpuTemp);

	// Serial.print("batt1Temp: ");
	// Serial.println(asvTelem.batt1Temp);
	// Serial.print("killStats: ");
	// Serial.println(asvTelem.killStats);
	// Serial.print("ctrlMode: ");
	// Serial.println(asvTelem.ctrlMode);
	// Serial.print("loader: ");
	// Serial.println(asvTelem.loader);
	// Serial.print("mhpbHB: ");
	// Serial.println(asvTelem.mhpbHB);
	// Serial.print("thHB: ");
	// Serial.println(asvTelem.thHB);
	// Serial.print("navHullTemp: ");
	// Serial.println(asvTelem.navHullTemp);
	// Serial.print("vertAct: ");
	// Serial.println(asvTelem.vertAct);
	// Serial.print("batt1Volt: ");
	// Serial.println(asvTelem.batt1Volt);
	// Serial.print("batt2Volt: ");
	// Serial.println(asvTelem.batt2Volt);
	// Serial.print("rcHB: ");
	// Serial.println(asvTelem.rcHB);
	// Serial.print("esc1HB: ");
	// Serial.println(asvTelem.esc1HB);
	// Serial.print("esc2HB: ");
	// Serial.println(asvTelem.esc2HB);
	// Serial.print("batt2HB: ");
	// Serial.println(asvTelem.batt2HB);
	// Serial.print("navHB: ");
	// Serial.println(asvTelem.navHB);
	// Serial.print("navHullHum: ");
	// Serial.println(asvTelem.navHullHum);

	// Serial.print("popbPcCurr: ");
	// Serial.println(asvTelem.popbPcCurr);
	// Serial.print("batt1Curr: ");
	// Serial.println(asvTelem.batt1Curr);
	// Serial.print("batt2Curr: ");
	// Serial.println(asvTelem.batt2Curr);

	// Serial.print("popbRestCurr: ");
	// Serial.println(asvTelem.popbRestCurr);
	// Serial.print("popbPcVolt: ");
	// Serial.println(asvTelem.popbPcVolt);
	// Serial.print("popbRestVolt: ");
	// Serial.println(asvTelem.popbRestVolt);
	// Serial.print("portBowAzi: ");
	// Serial.println(asvTelem.portBowAzi);
	// Serial.print("portQtrAzi: ");
	// Serial.println(asvTelem.portQtrAzi);
	// Serial.print("starBowAzi: ");
	// Serial.println(asvTelem.starBowAzi);
	// Serial.print("starQtrAzi: ");
	// Serial.println(asvTelem.starQtrAzi);
	// Serial.print("flyweel: ");
	// Serial.println(asvTelem.flyweel);
	// Serial.print("mainHullTemp: ");
	// Serial.println(asvTelem.mainHullTemp);
	// Serial.print("pcHb: ");
	// Serial.println(asvTelem.pcHb);
	// Serial.print("mainHullHum: ");
	// Serial.println(asvTelem.mainHullHum);
	// Serial.print("lbkpHB: ");
	// Serial.println(asvTelem.lbkpHB);
	// Serial.print("powerHullTemp: ");
	// Serial.println(asvTelem.powerHullTemp);
	// Serial.print("popbHB: ");
	// Serial.println(asvTelem.popbHB);
	// Serial.print("powerHullHum: ");
	// Serial.println(asvTelem.powerHullHum);
	// Serial.print("batt1HB: ");
	// Serial.println(asvTelem.batt1HB);
	// Serial.print("actHullTemp: ");
	// Serial.println(asvTelem.actHullTemp);
	// Serial.print("actHB: ");
	// Serial.println(asvTelem.actHB);

	// Serial.println(
	// 	"telemetry in sendable form =======================================================================");
	// Serial.print("latitude: ");
	// Serial.println(crsfTelem.crsf_gps.latitude);
	// Serial.print("longitude: ");
	// Serial.println(crsfTelem.crsf_gps.longitude);
	// Serial.print("groundSpeed: ");
	// Serial.println(crsfTelem.crsf_gps.groundSpeed);
	// Serial.print("gpsHeading: ");
	// Serial.println(crsfTelem.crsf_gps.gpsHeading);
	// Serial.print("altitude: ");
	// Serial.println(crsfTelem.crsf_gps.altitude);
	// Serial.print("satUsed: ");
	// Serial.println(crsfTelem.crsf_gps.satUsed);

	// Serial.print("batt_voltage: ");
	// Serial.println(crsfTelem.crsf_batt.batt_voltage);
	// Serial.print("batt_current: ");
	// Serial.println(crsfTelem.crsf_batt.batt_current);
	// Serial.print("fuel: ");
	// Serial.println(crsfTelem.crsf_batt.fuel);
	// Serial.print("battPercent: ");
	// Serial.println(crsfTelem.crsf_batt.battPercent);

	// Serial.print("pitch: ");
	// Serial.println(crsfTelem.crsf_attitude.pitch);
	// Serial.print("roll: ");
	// Serial.println(crsfTelem.crsf_attitude.roll);
	// Serial.print("yaw: ");
	// Serial.println(crsfTelem.crsf_attitude.yaw);

	// Serial.println("flightMode: ");
	// for (int i = 0; i < sizeof(crsfTelem.flightMode); i++) {
	// 	Serial.print(i);
	// 	Serial.print("\t");
	// 	Serial.print(crsfTelem.flightMode[i]);
	// 	Serial.print("\t");
	// 	Serial.println(crsfTelem.flightMode[i], HEX);
	// }

	// Serial.println("deciphering sendable form back to telem ==================================");

	// Serial.print("tilt: ");
	// Serial.println(crsfTelem.crsf_gps.latitude / 10000000.0);
	// Serial.print("pan: ");
	// Serial.println(crsfTelem.crsf_gps.longitude / 10000000.0);
	// Serial.print("cpuTemp: ");
	// Serial.println((crsfTelem.crsf_gps.groundSpeed & 0x00FF) >> 1);
	// Serial.print("mhpbEn: ");
	// Serial.println((crsfTelem.crsf_gps.groundSpeed & 0xFF00) >> 8, HEX);
	// Serial.print("actHullHum: ");
	// Serial.println((crsfTelem.crsf_gps.gpsHeading & 0x00FF) >> 1);
	// Serial.print("mhpbCycle: ");
	// Serial.println((crsfTelem.crsf_gps.gpsHeading & 0xFF00) >> 8, HEX);

	// Serial.print("batt2Temp: ");
	// Serial.println((crsfTelem.crsf_gps.altitude & 0x00FF) >> 1);
	// Serial.print("mhpbFault: ");
	// Serial.println((crsfTelem.crsf_gps.altitude & 0xFF00) >> 8, HEX);
	// Serial.print("gpuTemp: ");
	// Serial.println(crsfTelem.crsf_gps.satUsed >> 1);

	// Serial.print("batt1Temp: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x00FF) >> 1);
	// Serial.print("killStats: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x0300) >> 8);
	// Serial.print("ctrlMode: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x0C00) >> 10);
	// Serial.print("loader: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x3000) >> 12);
	// Serial.print("mhpbHB: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x4000) >> 14);
	// Serial.print("thHB: ");
	// Serial.println((crsfTelem.crsf_batt.batt_voltage & 0x8000) >> 15);
	// Serial.print("navHullTemp: ");
	// Serial.println((crsfTelem.crsf_batt.batt_current & 0x00FF) >> 1);
	// Serial.print("vertAct: ");
	// Serial.println((crsfTelem.crsf_batt.batt_current & 0xFF00) >> 8);
	// Serial.print("batt1Volt: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x0003FE) >> 1);
	// Serial.print("batt2Volt: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x07FC00) >> 10);
	// Serial.print("rcHB: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x080000) >> 19);
	// Serial.print("esc1HB: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x100000) >> 20);
	// Serial.print("esc2HB: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x200000) >> 21);
	// Serial.print("batt2HB: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x400000) >> 22);
	// Serial.print("navHB: ");
	// Serial.println((crsfTelem.crsf_batt.fuel & 0x800000) >> 23);
	// Serial.print("navHullHum: ");
	// Serial.println(crsfTelem.crsf_batt.battPercent >> 1);

	// Serial.print("popbPcCurr: ");
	// Serial.println(crsfTelem.crsf_attitude.pitch);
	// Serial.print("batt1Curr: ");
	// Serial.println(crsfTelem.crsf_attitude.roll);
	// Serial.print("batt2Curr: ");
	// Serial.println(crsfTelem.crsf_attitude.yaw);

	// Serial.print("popbRestCurr: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[0]));
	// Serial.print("popbPcVolt: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[1]));
	// Serial.print("popbRestVolt: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[2]));
	// Serial.print("portBowAzi: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[3]));
	// Serial.print("portQtrAzi: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[4]));
	// Serial.print("starBowAzi: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[5]));
	// Serial.print("starQtrAzi: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[6]));
	// Serial.print("flyweel: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[7]));
	// Serial.print("mainHullTemp: ");
	// Serial.println(uint8_t(crsfTelem.flightMode[8] & 0x7F));
	// Serial.print("pcHb: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[8] & 0x80) >> 7));
	// Serial.print("mainHullHum: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[9] & 0x7F)));
	// Serial.print("lbkpHB: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[9] & 0x80) >> 7));
	// Serial.print("powerHullTemp: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[10] & 0x7F)));
	// Serial.print("popbHB: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[10] & 0x80) >> 7));
	// Serial.print("powerHullHum: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[11] & 0x7F)));
	// Serial.print("batt1HB: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[11] & 0x80) >> 7));
	// Serial.print("actHullTemp: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[12] & 0x7F)));
	// Serial.print("actHB: ");
	// Serial.println((uint8_t(crsfTelem.flightMode[12] & 0x80) >> 7));
}

/**
 * @brief Get gimbal mode from RC
 * @param None
 * @retval RC_Gimbal_Mode::THRUSTERS_MODE or RC_Gimbal_Mode::BALLSHOOTER_MODE
 */
RC_Gimbal_Mode rcGetGimbalMode()
{
	// min                  mid                 max
	// AWAY                 MID                 TOWARDS
	// THRUSTERS_MODE       THRUSTERS_MODE      BALLSHOOTER_MODE

	if (rcGetTristateVal(GIMBAL_MODE_CH) == RC_Tristate::AWAY) {
		return RC_Gimbal_Mode::THRUSTERS_MODE;
	}
	else if (rcGetTristateVal(GIMBAL_MODE_CH) == RC_Tristate::MID) {
		return RC_Gimbal_Mode::THRUSTERS_MODE;
	}
	else {
		return RC_Gimbal_Mode::BALLSHOOTER_MODE;
	}
}

/**
 * @brief Get kill from RC
 * @param None
 * @retval RC_Kill::KILLED or RC_Kill::OPERATIONAL
 */
RC_Kill rcGetKill()
{
	// min      max
	// Away     Towards
	// KILLED   OPERATIONAL

	if (rcGetBistateVal(KILL_CH) == RC_Bistate::AWAY) {
		return RC_Kill::KILLED;
	}
	else {
		return RC_Kill::OPERATIONAL;
	}
}

/**
 * @brief Get vehicle control mode from RC
 * @param None
 * @retval RC_Vehicle_Mode::AUTONOMOUS or RC_Vehicle_Mode::RC_CTRL_MODE or RC_Vehicle_Mode::STATION_KEEP
 */
RC_Vehicle_Mode rcGetVehicleControlMode()
{
	// min              mid             max
	// AWAY             MID             TOWARDS
	// RC_CTRL_MODE     STATION_KEEP    AUTONOMOUS

	if (rcGetTristateVal(VEHICLE_MODE_CH) == RC_Tristate::AWAY) {
		return RC_Vehicle_Mode::RC_CTRL_MODE;
	}
	else if (rcGetTristateVal(VEHICLE_MODE_CH) == RC_Tristate::MID) {
		return RC_Vehicle_Mode::STATION_KEEP;
	}
	else {
		return RC_Vehicle_Mode::AUTONOMOUS;
	}
}

/**
 * @brief Get thruster deployment from RC
 * @param None
 * @retval RC_Thrust_Deployment::RETRACTED or RC_Thrust_Deployment::DEPLOYED
 */
RC_Thrust_Deployment rcGetThrustDeploy()
{
	// min          mid         max
	// AWAY         MID         TOWARDS
	// RETRACTED    RETRACTED   DEPLOYED

	if (rcGetTristateVal(THRUST_DEPLOY_CH) == RC_Tristate::AWAY) {
		return RC_Thrust_Deployment::RETRACTED;
	}
	else if (rcGetTristateVal(THRUST_DEPLOY_CH) == RC_Tristate::MID) {
		return RC_Thrust_Deployment::RETRACTED;
	}
	else {
		return RC_Thrust_Deployment::DEPLOYED;
	}
}

/**
 * @brief Get hydrophone deployment from RC
 * @param None
 * @retval RC_Hydro_Deployment::RETRACTED or RC_Hydro_Deployment::DEPLOYED
 */
RC_Hydro_Deployment rcGetHydroDeploy()
{
	// min          mid         max
	// AWAY         MID         TOWARDS
	// RETRACTED    RETRACTED   DEPLOYED

	if (rcGetTristateVal(HYDRO_DEPLOY_CH) == RC_Tristate::AWAY) {
		return RC_Hydro_Deployment::RETRACTED;
	}
	else if (rcGetTristateVal(HYDRO_DEPLOY_CH) == RC_Tristate::MID) {
		return RC_Hydro_Deployment::RETRACTED;
	}
	else {
		return RC_Hydro_Deployment::DEPLOYED;
	}
}

/**
 * @brief Get actuation lock from RC
 * @param None
 * @retval RC_Act_Lock::LOCKED or RC_Act_Lock::UNLOCKED
 */
RC_Act_Lock rcGetActLock()
{
	// min          mid         max
	// AWAY         MID         TOWARDS
	// LOCKED       LOCKED      UNLOCKED

	if (rcGetTristateVal(ACT_LOCK_CH) == RC_Tristate::AWAY) {
		return RC_Act_Lock::LOCKED;
	}
	else if (rcGetTristateVal(ACT_LOCK_CH) == RC_Tristate::MID) {
		return RC_Act_Lock::LOCKED;
	}
	else {
		return RC_Act_Lock::UNLOCKED;
	}
}

/**
 * @brief Get ballshoter load shoot from RC, load and shoot will only occur when switch is from min to max, max
 * to min is to reset
 * @param None
 * @retval RC_Load_Shoot::IDLE or RC_Load_Shoot::LOAD or RC_Load_Shoot::SHOOT
 */
RC_Load_Shoot rcGetLoadShoot()
{
	static RC_Load_Shoot currentState = RC_Load_Shoot::IDLE;

	if (rcGetTristateVal(LOAD_SHOOT_CH) == RC_Tristate::AWAY && (currentState == RC_Load_Shoot::SHOOT)) {
		currentState = RC_Load_Shoot::IDLE;
	}

	if (rcGetTristateVal(LOAD_SHOOT_CH) == RC_Tristate::MID && (currentState == RC_Load_Shoot::IDLE)) {
		currentState = RC_Load_Shoot::LOAD;
	}

	if (rcGetTristateVal(LOAD_SHOOT_CH) == RC_Tristate::TOWARDS && (currentState == RC_Load_Shoot::LOAD)) {
		currentState = RC_Load_Shoot::SHOOT;
	}

	return currentState;
}

/**
 * @brief Get calibration select from RC
 * @param None
 * @retval RC_Act_Cal indicating which actuator to calibrate
 */
RC_Act_Cal rcGetActCal()
{
	uint16_t raw = crsf.getChannel(ACT_CAL_CH);
	RC_Act_Cal cal;

	switch (raw) {
	case CAL_PB_AZI_RC_VAL: {
		cal = RC_Act_Cal::PB_AZI;
		break;
	}
	case CAL_PQ_AZI_RC_VAL: {
		cal = RC_Act_Cal::PQ_AZI;
		break;
	}
	case CAL_SB_AZI_RC_VAL: {
		cal = RC_Act_Cal::SB_AZI;
		break;
	}
	case CAL_SQ_AZI_RC_VAL: {
		cal = RC_Act_Cal::SQ_AZI;
		break;
	}
	case CAL_PAN_RC_VAL: {
		cal = RC_Act_Cal::PAN;
		break;
	}
	case CAL_TILT_RC_VAL: {
		cal = RC_Act_Cal::TILT;
		break;
	}
	case CAL_PB_VERT_RC_VAL: {
		cal = RC_Act_Cal::PB_VERT;
		break;
	}
	case CAL_PQ_VERT_RC_VAL: {
		cal = RC_Act_Cal::PQ_VERT;
		break;
	}
	case CAL_SB_VERT_RC_VAL: {
		cal = RC_Act_Cal::SB_VERT;
		break;
	}
	case CAL_SQ_VERT_RC_VAL: {
		cal = RC_Act_Cal::SQ_VERT;
		break;
	}
	case CAL_HY_VERT_RC_VAL: {
		cal = RC_Act_Cal::HY_VERT;
		break;
	}
	case CAL_IDLE_RC_VAL: {
		cal = RC_Act_Cal::IDLE;
		break;
	}
	default: {
		cal = RC_Act_Cal::IDLE;
		break;
	}
	};

	return cal;
}

/**
 * @brief Get thruster azimuth from RC
 * @param None
 * @retval RC_Thruster_Azi::STATION_KEEP or RC_Thruster_Azi::UNDER_WAY
 */
RC_Thruster_Azi rcGetThrustAziMode()
{
	uint16_t raw = crsf.getChannel(THRUST_AZI_CH);

	switch (raw) {
	case TH_AZI_0_RC_VAL:
		return RC_Thruster_Azi::STATION_KEEP;
		break;
	case TH_AZI_1_RC_VAL:
		return RC_Thruster_Azi::UNDER_WAY;
		break;
	case TH_AZI_2_RC_VAL:
		return RC_Thruster_Azi::UNDER_WAY;
		break;
	case TH_AZI_3_RC_VAL:
		return RC_Thruster_Azi::UNDER_WAY;
		break;
	case TH_AZI_4_RC_VAL:
		return RC_Thruster_Azi::UNDER_WAY;
		break;
	case TH_AZI_5_RC_VAL:
		return RC_Thruster_Azi::UNDER_WAY;
		break;
	default:
		return RC_Thruster_Azi::STATION_KEEP;
		break;
	}
}

/**
 * @brief Get main hull power bus cycle control from RC
 * @param None
 * @retval uint8_t indicating which channel to cycle
 */
uint8_t rcGetMHPBCycle()
{
	uint16_t raw = crsf.getChannel(MHPB_CYCLE_CH);
	uint8_t cycle = 0x00;

	switch (raw) {
	case MHPB_PSU_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_PSU;
		break;
	}
	case MHPB_NAV_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_NAV_HULL;
		break;
	}
	case MHPB_EXT_24V_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_EXTRA_24V;
		break;
	}
	case MHPB_OUST_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_OUSTER;
		break;
	}
	case MHPB_EXT_5V_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_EXTRA_5V;
		break;
	}
	case MHPB_FANS_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_PUMP;
		break;
	}
	case MHPB_LBKP_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_LOGIC;
		break;
	}
	case MHPB_ETH_RC_VAL: {
		cycle = mhpb_power_ctrl_rc::CYCLE_SWITCH;
		break;
	}
	default: {
		cycle = 0x00;
		break;
	}
	};

	return cycle;
}

/**
 * @brief Get ballshooter pan, tilt and flywheel speed from RC to a CAN standard value
 * @param canPan pan value for ballshooter in CAN
 * @param canTilt tilt value for ballshooter in CAN
 * @param canFlywheel flywheel speed value for ballshooter in CAN
 * @retval None
 */
void rcGetBallshooterVal(uint16_t& canPan, uint16_t& canTilt, uint16_t& canFlywheel)
{
	// get channel values
	uint16_t rcPan = rcGetStickMidDeadZone(YAW_PAN_CH);
	uint16_t rcTilt = rcGetStickMidDeadZone(SURGE_TILT_CH);
	uint16_t rcFlywheel = rcGetStickMinDeadZone(FLYWHEEL_CH);

	// pan and tilt are incremnetal, bigger gimbal values means increment faster
	static uint16_t panPos = BALLSHOOTER_0_DEG;
	static uint16_t tiltPos = BALLSHOOTER_0_DEG;

	// handle for pan
	if (rcPan == CHANNEL_MID_VAL) {
		canPan = rc_ballshooter_ctrl::PAN_IGNORE;
	}
	else if (rcPan < CHANNEL_MID_VAL) {
		if (panPos > rc_ballshooter_ctrl::PAN_LEFT) {
			int16_t panDec = map(rcPan, CHANNEL_MIN_VAL, CHANNEL_MID_VAL, -5, 0);

			// check for overflow
			if (panPos < uint16_t((int)rc_ballshooter_ctrl::PAN_LEFT - panDec)) {
				panPos = rc_ballshooter_ctrl::PAN_LEFT;
			}
			else {
				panPos += panDec;
			}
		}
		else {
			panPos = rc_ballshooter_ctrl::PAN_LEFT;
		}

		canPan = panPos;
	}
	else {
		if (panPos < rc_ballshooter_ctrl::PAN_RIGHT) {
			int16_t panInc = map(rcPan, CHANNEL_MID_VAL, CHANNEL_MAX_VAL, 0, 5);

			// check for overflow
			if (panPos > uint16_t((int)rc_ballshooter_ctrl::PAN_RIGHT - panInc)) {
				panPos = rc_ballshooter_ctrl::PAN_RIGHT;
			}
			else {
				panPos += panInc;
			}
		}
		else {
			panPos = rc_ballshooter_ctrl::PAN_RIGHT;
		}

		canPan = panPos;
	}

	// handle for tilt
	if (rcTilt == CHANNEL_MID_VAL) {
		canTilt = rc_ballshooter_ctrl::TILT_IGNORE;
	}
	else if (rcTilt < CHANNEL_MID_VAL) {
		if (tiltPos > rc_ballshooter_ctrl::TILT_DOWN) {
			int16_t tiltDec = map(rcTilt, CHANNEL_MIN_VAL, CHANNEL_MID_VAL, -5, 0);

			// check for overflow
			if (tiltPos < uint16_t((int)rc_ballshooter_ctrl::TILT_DOWN - tiltDec)) {
				tiltPos = rc_ballshooter_ctrl::TILT_DOWN;
			}
			else {
				tiltPos += tiltDec;
			}
		}
		else {
			tiltPos = rc_ballshooter_ctrl::TILT_DOWN;
		}

		canTilt = tiltPos;
	}
	else {
		if (tiltPos < rc_ballshooter_ctrl::TILT_UP) {
			int16_t tiltInc = map(rcTilt, CHANNEL_MID_VAL, CHANNEL_MAX_VAL, 0, 5);

			// check for overflow
			if (tiltPos > uint16_t((int)rc_ballshooter_ctrl::TILT_UP - tiltInc)) {
				tiltPos = rc_ballshooter_ctrl::TILT_UP;
			}
			else {
				tiltPos += tiltInc;
			}
		}
		else {
			tiltPos = rc_ballshooter_ctrl::TILT_UP;
		}

		canTilt = tiltPos;
	}

	// map to values for flywheel
	canFlywheel = map(rcFlywheel,
	                  CHANNEL_MIN_VAL,
	                  CHANNEL_MAX_VAL,
	                  rc_ballshooter_ctrl::FLYWHEEL_OFF,
	                  rc_ballshooter_ctrl::FLYWHEEL_FORWARD_MAX);
}

/**
 * @brief Get movement for surge, sway and yaw from RC sticks and map to 0 - 6400 for the four thrusters in CAN
 * @param canPortBow CAN value to be sent for port bow thruster
 * @param canStarBow CAN value to be sent for starboard bow thruster
 * @param canPortQtr CAN value to be sent for port quarter thruster
 * @param canStarQtr CAN value to be sent for starboard quarter thruster
 * @retval None
 */
void rcGetThrusterVal(uint16_t& canPortBow, uint16_t& canStarBow, uint16_t& canPortQtr, uint16_t& canStarQtr)
{
	// get channel values
	uint16_t yaw_rc = rcGetStickMidDeadZone(YAW_PAN_CH);
	uint16_t surge_rc = rcGetStickMidDeadZone(SURGE_TILT_CH);
	uint16_t sway_rc = rcGetStickMidDeadZone(SWAY_CH);

	// map to -3200, 3200, (int) to type cast to int to get negative values
	int16_t yaw_val = map(yaw_rc,
	                      CHANNEL_MIN_VAL,
	                      CHANNEL_MAX_VAL,
	                      (int)thruster_ctrl::REVERSE_MAX - (int)thruster_ctrl::STOP,
	                      (int)thruster_ctrl::FORWARD_MAX - (int)thruster_ctrl::STOP);
	int16_t surge_val = map(surge_rc,
	                        CHANNEL_MIN_VAL,
	                        CHANNEL_MAX_VAL,
	                        (int)thruster_ctrl::REVERSE_MAX - (int)thruster_ctrl::STOP,
	                        (int)thruster_ctrl::FORWARD_MAX - (int)thruster_ctrl::STOP);
	int16_t sway_val = map(sway_rc,
	                       CHANNEL_MIN_VAL,
	                       CHANNEL_MAX_VAL,
	                       (int)thruster_ctrl::REVERSE_MAX - (int)thruster_ctrl::STOP,
	                       (int)thruster_ctrl::FORWARD_MAX - (int)thruster_ctrl::STOP);

	// translate to trusters vectors and 0, 6400
	if (rcGetThrustAziMode() == RC_Thruster_Azi::STATION_KEEP) {
		// arrow points to direction of thrusters positive and direction of force on ASV/opposite of water
		// flow, look from cage side if anti c/w is forward
		// angle is from for aft axis
		// 1 port bow 45° ↗   ↖ 45° star bow 3
		//                 ASV
		// 2 port qtr 45° ↖   ↗ 45° star qtr 4
		canPortBow = constrain(surge_val + sway_val + yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canStarBow = constrain(surge_val - sway_val - yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canPortQtr = constrain(surge_val - sway_val + yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canStarQtr = constrain(surge_val + sway_val - yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
	}
	else {
		// arrow points to direction of thrusters positive and direction of force on ASV/opposite of water
		// flow, look from cage side if anti c/w is forward
		// angle is from for aft axis
		// 1 port bow 30° ↗   ↖ 30° star bow 3
		//                 ASV
		// 2 port qtr     ↑   ↑     star qtr 4

		// TODO: add scaling so its easier to drive?
		canPortBow = constrain(surge_val + sway_val + yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canStarBow = constrain(surge_val - sway_val - yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canPortQtr = constrain(surge_val - sway_val + yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
		canStarQtr = constrain(surge_val + sway_val - yaw_val + (int)thruster_ctrl::STOP,
		                       (int)thruster_ctrl::REVERSE_MAX,
		                       (int)thruster_ctrl::FORWARD_MAX);
	}
}

/**
 * @brief remove dead zone at the minimum end and constrain to CH min and max for stick
 * @param channel
 * @retval cleaned up channel value
 */
uint16_t rcGetStickMinDeadZone(uint8_t channel)
{
	uint16_t raw = crsf.getChannel(channel);

	// remove dead zone
	if (raw < CHANNEL_MIN_DEADZONE_TH) {
		return CRSF_CHANNEL_MIN;
	}

	// map to start after deadzone, so its a smoother transition
	return map(constrain(raw, CHANNEL_MIN_VAL, CHANNEL_MAX_VAL),
	           CHANNEL_MIN_DEADZONE_TH,
	           CHANNEL_MAX_VAL,
	           CHANNEL_MIN_VAL,
	           CHANNEL_MAX_VAL);
}

/**
 * @brief remove dead zone at the middle and constrain to CH min and max for stick
 * @param channel
 * @retval cleaned up channel value
 */
uint16_t rcGetStickMidDeadZone(uint8_t channel)
{
	uint16_t raw = crsf.getChannel(channel);

	// remove dead zone
	if (raw > CHANNEL_MID_DEADZONE_LOWER_TH && raw < CHANNEL_MID_DEADZONE_UPPER_TH) {
		return CHANNEL_MID_VAL;
	}

	// map to start after deadzone, so its a smoother transition
	if (raw < CHANNEL_MID_DEADZONE_LOWER_TH) {
		return map(constrain(raw, CHANNEL_MIN_VAL, CHANNEL_MAX_VAL),
		           CHANNEL_MIN_VAL,
		           CHANNEL_MID_DEADZONE_LOWER_TH,
		           CHANNEL_MIN_VAL,
		           CHANNEL_MID_VAL);
	}
	else {
		return map(constrain(raw, CHANNEL_MIN_VAL, CHANNEL_MAX_VAL),
		           CHANNEL_MID_DEADZONE_UPPER_TH,
		           CHANNEL_MAX_VAL,
		           CHANNEL_MID_VAL,
		           CHANNEL_MAX_VAL);
	}
}

/**
 * @brief read bistate switch and threshold to RC_Bistate::AWAY or RC_Bistate::TOWARDS
 * @param channel
 * @retval RC_Bistate::AWAY or RC_Bistate::TOWARDS
 */
RC_Bistate rcGetBistateVal(uint8_t channel)
{
	uint16_t raw = crsf.getChannel(channel);

	if (raw < BISTATE_TH) {
		return RC_Bistate::AWAY;
	}
	else {
		return RC_Bistate::TOWARDS;
	}
}

/**
 * @brief read tristate switch and threshold to RC_Tristate::AWAY or RC_Tristate::MID or RC_Tristate::TOWARDS
 * @param channel
 * @retval RC_Tristate::AWAY or RC_Tristate::MID or RC_Tristate::TOWARDS
 */
RC_Tristate rcGetTristateVal(uint8_t channel)
{
	uint16_t raw = crsf.getChannel(channel);

	if (raw < TRISTATE_LOW_TH) {
		return RC_Tristate::AWAY;
	}
	else if (raw > TRISTATE_HIGH_TH) {
		return RC_Tristate::TOWARDS;
	}
	else {
		return RC_Tristate::MID;
	}
}
