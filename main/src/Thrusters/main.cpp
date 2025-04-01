// ######################################################
//
// ____________________    _____    _____________   ____
// \______   \______   \  /  _  \  /   _____/\   \ /   /
//  |    |  _/|    |  _/ /  /_\  \ \_____  \  \   Y   /
//  |    |   \|    |   \/    |    \/        \  \     /
//  |______  /|______  /\____|__  /_______  /   \___/
//         \/        \/         \/        \/
//
// Thrusters Board for ASV 4.0
//
// Written By: Justin3
//
// Summary:
// - Control signal to ESC based on RC status
// -- Soft kill for ESC
// -- Publish ESC Power Control
// - Control light tower
// - Read and publish Main Hull internal stats
// - Read and publish ESC/Thruster stats
//
// ** NO CHANGES TO ASV3.5 BOARD, FIRMWARE PORTED OVER **
// ######################################################

#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <can.h>

#include <RobotEQ.h>
#include <Adafruit_SHT4x.h>

#include "Thrusters/define.h"
#include "Thrusters/main.h"

#include "common/bb_can.h"
using namespace bb;

//===========================================
//
//        GLOBAL DECLARATIONS
//
//===========================================
// CAN Variables
MCP_CAN can(CAN_CHIP_SELECT);
can_id_t id = 0;
uint8_t len = 0;
uint8_t buf[8];

// Thrusters Data Variables
Roboteq esc1(&can, 1);
Roboteq esc2(&can, 2);
uint8_t control_mode = 0; // 1 - auton, 2 - rc, 3 - stationkeep

bool esc_heartbeat[ESC_COUNT] = {0};
uint32_t esc_heartbeat_timers[ESC_COUNT] = {0};

// Init all true kill as killed
// To init as SBC unkill, replace bitstring with 0b001101
uint8_t kill_status = 0b001111; // {0 = Hard, 1 = SBC, 2 = RC, 3 = Lost Telem, 4 = ESC1, 5 = ESC2} BITSTRING IS IN REVERSE
uint8_t kill_flag = 0b11; // {0 = ESC1, 1 = ESC2}, Init as all killed
int16_t thruster_data[THRUSTER_COUNT] = {0};
int16_t thruster_stats[THRUSTER_COUNT] = {0};
uint16_t esc_fault_flags[ESC_COUNT] = {0};
uint16_t motor_status_flags[THRUSTER_COUNT] = {0};
uint32_t kill_timers[KILL_TYPE_COUNT] = {0};
// uint8_t motor_status_channel[ESC_COUNT] = {1, 1}; // We can use this to implement getting motor current alternately from each ESC

// Light Tower Variables
const int LIGHT_PINS[LIGHT_GATE_COUNT] = {GREEN_GATE, YELLOW_GATE, RED_GATE};
uint8_t light_state = 2; // 0 - auton, 1 - rc, 2 - kill, 3 - stationkeep, default kill
uint32_t blink_timer;
bool blink_state = false;

// Sensors Variables
Adafruit_SHT4x sht = Adafruit_SHT4x();
sensors_event_t humidity, temp;

const int TEMP_PROBE_PINS[TEMP_PROBE_COUNT] = {TP1_PIN, TP2_PIN, TP3_PIN};
int int_stats[STAT_COUNT] = {0};

// CAN Callback Timers (Pub)
uint32_t can_timers[CAN_ID_COUNT] = {0};

// Sensor Callback Timers (Read)
uint32_t sensor_timers[SENSOR_TYPE_COUNT] = {0};
uint8_t sensors_validity = 0b00; // {0 = SHT, 1 = Temp Probe}

//===========================================
//
//        SETUP & LOOP
//
//===========================================
void setup() {
    Serial.begin(115200);
    Serial.println("Thrusters Board Init Begin");

    // CAN Init
    pinMode(CAN_CHIP_SELECT, OUTPUT);
    digitalWrite(CAN_CHIP_SELECT, HIGH);
    // CAN Timers Init
    for (int i = 0; i < CAN_ID_COUNT; i++) {
        can_timers[i] = millis();
    }

    can_init();
    setCANMask();
    Serial.println("CAN OK");

    // ESC HB Timer Init
    for (int i = 0; i < ESC_COUNT; i++) {
        esc_heartbeat_timers[i] = millis();
    }

    // Hard Kill Init
    pinMode(HARD_KILL_PIN, INPUT);

    // Light Tower Init
    light_tower_init();
    blink_timer = millis();

    // I2C Init
    Wire.begin();

    // Sensors Init
    SHT_init();
    // Sensors Timer Init
    for (int i = 0; i < SENSOR_TYPE_COUNT; i++) {
        sensor_timers[i] = millis();
    }

    Serial.println("Peripherals OK");

    Serial.println("Thrusters Board Init Complete");
}

void loop() {
    sensors_read();
    update_timeout();

    control_kill();
    control_light_tower();

    publishCAN();
    checkCANmsg();
}

//===========================================
//
//        CAN FUNCTIONS
//
//===========================================

void can_init() {
START_INIT:
	if (CAN_OK == can.begin(CAN_1000KBPS)) { // init can bus : baudrate = 1000k
#ifdef BB_DEBUG
		Serial.println("CAN init ok!");
#endif // BB_DEBUG
	}
	else {
#ifdef BB_DEBUG
		Serial.println("CAN init fail");
		Serial.println("Init CAN again");
		delay(1000);
#endif // BB_DEBUG
		goto START_INIT;
	}
}

void setCANMask() {
    //mask register 0
    can.init_Mask(0, 0, can_mask::THRUSTERS.mask); 
    can.init_Filt(0, 0, can_mask::THRUSTERS.filter);
}

void checkCANmsg() {
	if (CAN_MSGAVAIL == can.checkReceive()) {
		can.readMsgBufID(&id, &len, buf); // read data, len: data length, buf: data buf

// #ifdef BB_DEBUG
//         Serial.print("Can id: ");
//         Serial.println(can.getCanId());

//         Serial.print("Data: ");
//         for (int i = 0; i < len; i++) {
//             Serial.print(buf[i]);
//             Serial.print(" ");
//         }
//         Serial.println();
// #endif

        switch (can.getCanId()) {
            case can_id::SOFT_KILL: {
                uint8_t origin;
                if (buf[1] == SOFT_KILL_SOURCE_SBC) {
                    // SBC kill override when in RC mode
                    if (control_mode == BB_CTRL_MODE_RC) {
                        break;
                    }
                    origin = SBC_SOFT_KILL_IDX;
                }
                else if (buf[1] == SOFT_KILL_SOURCE_RC) {
                    origin = RC_SOFT_KILL_IDX;
                }

                // Killed
                if (buf[0]) {
                    kill_status |= (1 << origin);
                    kill_timers[origin] = 0;
                }
                // Operational
                else {
                    kill_status &= ~(1 << origin);
                    kill_timers[origin] = millis();
                }

                break;
            }

            case can_id::CTRL_LINK: {
                control_mode = buf[0];

                // SBC kill override when in RC mode
                if (control_mode == BB_CTRL_MODE_RC) {
                    kill_status &= ~(1 << SBC_SOFT_KILL_IDX);
                    kill_timers[SBC_SOFT_KILL_IDX] = millis();
                }

                break;
            }

            case can_id::RC_THRUSTER: {
                if (control_mode == BB_CTRL_MODE_RC) {
                    for (int i = 0; i < THRUSTER_COUNT; i++) {
                        int thruster_speed = can.parseCANFrame(buf, i * 2, 2);
                        int speed = map(thruster_speed, 0, 6400, -1000, 1000);
                        speed = constrain(speed, -1000, 1000);
                        thruster_data[i] = speed;
                    }
                    publishThrusters();
                }
                kill_status &= ~(1 << TELEM_HB_KILL_IDX);
                kill_timers[TELEM_HB_KILL_IDX] = millis();
                break;
            }

            case can_id::PC_THRUSTER: {
                if (control_mode == BB_CTRL_MODE_AUTO || control_mode == BB_CTRL_MODE_STATION_KEEP) {
                    for (int i = 0; i < THRUSTER_COUNT; i++) {
                        int thruster_speed = can.parseCANFrame(buf, i * 2, 2);
                        int speed = map(thruster_speed, 0, 6400, -1000, 1000);
                        speed = constrain(speed, -1000, 1000);
                        thruster_data[i] = speed;
                    }
                    publishThrusters();
                }
                break;
            }

            case can_id::HARD_KILL: {
                if (buf[0]) {
                    kill_status |= (1 << HARD_KILL_IDX);
                    kill_timers[HARD_KILL_IDX] = 0;
                }
                else {
                    kill_status &= ~(1 << HARD_KILL_IDX);
                    kill_timers[HARD_KILL_IDX] = millis();
                }
                break;
            }

            case can_id::ESC_CTRL: {
                int target_esc = buf[1];

                if (target_esc == 1) {
                    if (buf[0]) {
                        kill_status |= (1 << ESC1_KILL_IDX);
                        kill_timers[ESC1_KILL_IDX] = 0;
                    }
                    else {
                        kill_status &= ~(1 << ESC1_KILL_IDX);
                        kill_timers[ESC1_KILL_IDX] = millis();
                    }
                }
                else if (target_esc == 2) {
                    if (buf[0]) {
                        kill_status |= (1 << ESC2_KILL_IDX);
                        kill_timers[ESC2_KILL_IDX] = 0;
                    }
                    else {
                        kill_status &= ~(1 << ESC2_KILL_IDX);
                        kill_timers[ESC2_KILL_IDX] = millis();
                    }
                }
                break;
            }

            case ROBOTEQ_CAN1_REPLY_INDEX: {
                esc1.readRoboteqReply(id, len, buf);
                esc_heartbeat[ESC1_IDX] = true;
                esc_heartbeat_timers[ESC1_IDX] = millis();
                break;
            }

            case ROBOTEQ_CAN2_REPLY_INDEX: {
                esc2.readRoboteqReply(id, len, buf);
                esc_heartbeat[ESC2_IDX] = true;
                esc_heartbeat_timers[ESC2_IDX] = millis();
                break;
            }

            default: {
                break;
            }
        }
        can.clearMsg();
    }
}

// Publish thruster data and heartbeat + internal stats
void publishCAN() {
    // Heartbeat
    if (millis() - can_timers[HEARTBEAT_TIMER_IDX] > HEARTBEAT_LOOP) {
        publish_heartbeats();
        can_timers[HEARTBEAT_TIMER_IDX] = millis();
    }

    // Kill Status
    if (millis() - can_timers[KILL_STATUS_TIMER_IDX] > KILL_STATUS_LOOP) {
        publishKill_Status();
        can_timers[KILL_STATUS_TIMER_IDX] = millis();
    }

    // ESC Stats
    if (millis() - can_timers[ESC1_STATS_TIMER_IDX] > ESC_STATS_LOOP) {
        requestESC_Stats(1);
        publishESC_Stats(1);
        can_timers[ESC1_STATS_TIMER_IDX] = millis();
    }
    if (millis() - can_timers[ESC2_STATS_TIMER_IDX] > ESC_STATS_LOOP) {
        requestESC_Stats(2);
        publishESC_Stats(2);
        can_timers[ESC2_STATS_TIMER_IDX] = millis();
    }

    // ESC Flags
    if (millis() - can_timers[ESC1_FLAGS_TIMER_IDX] > ESC_FLAGS_LOOP) {
        requestMotor_Status_Flags(1);
        requestESC_Fault_Flags(1);
        publishESC_Flags(1);
        can_timers[ESC1_FLAGS_TIMER_IDX] = millis();
    }
    if (millis() - can_timers[ESC2_FLAGS_TIMER_IDX] > ESC_FLAGS_LOOP) {
        requestMotor_Status_Flags(2);
        requestESC_Fault_Flags(2);
        publishESC_Flags(2);
        can_timers[ESC2_FLAGS_TIMER_IDX] = millis();
    }

    // MH Stats
    if (millis() - can_timers[MH_STATS_TIMER_IDX] > MH_STATS_LOOP) {
        publishMH_Stats();
        can_timers[MH_STATS_TIMER_IDX] = millis();
    }
}

// Publish heartbeat id
void publishCAN_heartbeat(int device_id) {
	id = can_id::HEARTBEAT;
	len = 1;
	buf[0] = device_id;
	can.sendMsgBuf(can_id::HEARTBEAT, 0, len, buf);
}

void publish_heartbeats() {
    publishCAN_heartbeat(heartbeat_id::THRUSTERS_BOARD);
    // ESC Heartbeat
    if (esc_heartbeat[ESC1_IDX]) {
        publishCAN_heartbeat(heartbeat_id::ESC1);
    }
    if (esc_heartbeat[ESC2_IDX]) {
        publishCAN_heartbeat(heartbeat_id::ESC2);
    }
}

// Publish kill status
void publishKill_Status() {
    id = can_id::KILL_STATUS;
    len = 1;
    buf[0] = kill_status;
    can.sendMsgBuf(id, 0, len, buf);
}

// Reqeust ESC stats
void requestESC_Stats(int esc_id) {
    if (esc_id == 1) {
        esc1.requestMotorAmps(1);
        esc1.requestMotorAmps(2);
    }
    else if (esc_id == 2) {
        esc2.requestMotorAmps(1);
        esc2.requestMotorAmps(2);
    }
}

// Publish ESC stats
void publishESC_Stats(int esc_id) {
    len = 4;
    
    RoboteqStats esc_stats;
    int16_t motor_current1, motor_current2;
    if (esc_id == 1) {
        id = can_id::ESC1_STATS;
        esc_stats = esc1.getRoboteqStats();
        motor_current1 = esc_stats.motor_current1;
        motor_current2 = esc_stats.motor_current2;

        thruster_stats[0] = motor_current1;
        thruster_stats[1] = motor_current2;
    }
    else if (esc_id == 2) {
        id = can_id::ESC2_STATS;
        esc_stats = esc2.getRoboteqStats();
        motor_current1 = esc_stats.motor_current1;
        motor_current2 = esc_stats.motor_current2;

        thruster_stats[2] = motor_current1;
        thruster_stats[3] = motor_current2;
    }

    can.setupCANFrame(buf, 0, 2, motor_current1);
    can.setupCANFrame(buf, 2, 2, motor_current2);

    can.sendMsgBuf(id, 0, len, buf);
}

// Request ESC Fault flags
void requestESC_Fault_Flags(int esc_id) {
    if (esc_id == 1) {
        esc1.requestFaultFlags();
    }
    else if (esc_id == 2) {
        esc2.requestFaultFlags();
    }
}

// Request Motor Status flags
void requestMotor_Status_Flags(int esc_id) {
    if (esc_id == 1) {
        esc1.requestMotorStatusFlags(1);
        esc1.requestMotorStatusFlags(2);
    }
    else if (esc_id == 2) {
        esc2.requestMotorStatusFlags(1);
        esc2.requestMotorStatusFlags(2);
    }
}

// Publish ESC flags
void publishESC_Flags(int esc_id) {
    len = 6;

    RoboteqStats esc_stats;
    uint16_t motor1_status_flags, motor2_status_flags, fault_flags;
    if (esc_id == 1) {
        id = can_id::ESC1_MOTOR_FLAGS;
        esc_stats = esc1.getRoboteqStats();
        motor1_status_flags = esc_stats.motor_status_flags1;
        motor2_status_flags = esc_stats.motor_status_flags2;
        fault_flags = esc_stats.fault_flags;

        motor_status_flags[0] = motor1_status_flags;
        motor_status_flags[1] = motor2_status_flags;
        esc_fault_flags[0] = fault_flags;
    }
    else if (esc_id == 2) {
        id = can_id::ESC2_MOTOR_FLAGS;
        esc_stats = esc2.getRoboteqStats();
        motor1_status_flags = esc_stats.motor_status_flags1;
        motor2_status_flags = esc_stats.motor_status_flags2;
        fault_flags = esc_stats.fault_flags;

        motor_status_flags[2] = motor1_status_flags;
        motor_status_flags[3] = motor2_status_flags;
        esc_fault_flags[1] = fault_flags;
    }
    
    can.setupCANFrame(buf, 0, 2, motor1_status_flags);
    can.setupCANFrame(buf, 2, 2, motor2_status_flags);
    can.setupCANFrame(buf, 4, 2, fault_flags);

    can.sendMsgBuf(id, 0, len, buf);

    // Publish ESC E-Stop Flag to POPB
    bool e_stop_flag = fault_flags & (1 << E_STOP_BIT);
    if (e_stop_flag && kill_flag) {
        id = can_id::CONTACTOR_CTRL;
        len = 2;

        buf[0] = e_stop_flag;
        buf[1] = esc_id;

        can.sendMsgBuf(id, 0, len, buf);
    }
}

// Publish internal stats
void publishMH_Stats() {
	if (sensors_validity != 0b11)
		return;

    id = can_id::MAINHULL_STATS;
    len = 7;

    for (int i = 0; i < TEMP_PROBE_IDX_OFFSET; i++) {
        can.setupCANFrame(buf, i * 2, 2, int_stats[i]);
    }

    for (int i = 0; i < TEMP_PROBE_COUNT; i++) {    // Temp Probe
        int can_idx = TEMP_PROBE_CAN_OFFSET + i;
        int probe_idx = TEMP_PROBE_IDX_OFFSET + i;
        can.setupCANFrame(buf, can_idx, 1, int_stats[probe_idx]);
    }

    can.sendMsgBuf(id, 0, len, buf);
}

// Publish thruster data
void publishThrusters() {
    esc1.setMotorSpeed(thruster_data[0], 1);
    esc1.setMotorSpeed(thruster_data[1], 2);
    esc2.setMotorSpeed(thruster_data[2], 1);
    esc2.setMotorSpeed(thruster_data[3], 2);
}

//==========================================
//
//        LIGHT TOWER FUNCTIONS
//
//==========================================
void light_tower_init() {
    for (int i = 0; i < LIGHT_GATE_COUNT; i++) {
        pinMode(LIGHT_PINS[i], OUTPUT);
        digitalWrite(LIGHT_PINS[i], LOW);
    }
}

/*
GREEN           -   Auton
YELLOW BLINK    -   Station Keep
YELLOW          -   RC
RED             -   Kill
*/
void control_light_tower() {
    for (int i = 0; i < LIGHT_GATE_COUNT; i++) {
        digitalWrite(LIGHT_PINS[i], LOW);
    }

    // if (light_state != LIGHT_AUTON && light_state != LIGHT_STATIONKEEP) {
    if (light_state != LIGHT_STATIONKEEP) {
        digitalWrite(LIGHT_PINS[light_state], HIGH);
    }
    else {
        digitalWrite(LIGHT_PINS[LIGHT_RC], blink_state);
    }

    // Light Blink Timer
    if (millis() - blink_timer > LIGHT_BLINK_TIME) {
        blink_state = !blink_state;
        blink_timer = millis();
    }
}

//==========================================
//
//        SENSORS FUNCTIONS
//
//==========================================
void SHT_init() {
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

void SHT_read() {
    if (millis() - sensor_timers[SHT_TIMER_IDX] > SHT_LOOP) {
        if (sht.getEvent(&humidity, &temp)) {
            // We only want 1dp precision
            int_stats[SHT_TEMP_IDX] = temp.temperature * 10;
            int_stats[SHT_HUMIDITY_IDX] = humidity.relative_humidity * 10;
            sensor_timers[SHT_TIMER_IDX] = millis();
        
// #ifdef BB_DEBUG
//             // SHT precision 2dp for both temp and humidity
//             Serial.print("Temperature: ");
//             Serial.print(temp.temperature);
//             Serial.println(" *C");
//             Serial.print("Humidity: ");
//             Serial.print(humidity.relative_humidity);
//             Serial.println(" %");
// #endif // BB_DEBUG
        }
    }
}

void probe_read() {
    if (millis() - sensor_timers[TEMP_PROBE_TIMER_IDX] > TEMP_PROBE_LOOP) {
        for (int i = 0; i < TEMP_PROBE_COUNT; i++) {
            long long raw = analogRead(TEMP_PROBE_PINS[i]);
            double probe_temp = 172.015 - 0.3424061 * raw + 0.0003908044 * pow(raw, 2) - 2.247988e-7 * pow(raw, 3);
            int_stats[i + TEMP_PROBE_IDX_OFFSET] = probe_temp;
        }
        sensor_timers[TEMP_PROBE_TIMER_IDX] = millis();

// #ifdef BB_DEBUG
//         Serial.print("Temp Probes: ");
//         for (int i = 0; i < TEMP_PROBE_COUNT; i++) {
//             Serial.print(int_stats[i + TEMP_PROBE_IDX_OFFSET]);
//             Serial.print(" ");
//         }
//         Serial.println();
// #endif // BB_DEBUG
    }
}

void sensors_read() {
    SHT_read();
    probe_read();
}


//==========================================
//
//        KILL SWITCH FUNCTIONS
//
//==========================================
void control_kill () {
    // Check Hard Kill, Uncomment if we want to use hard kill onboard
    /*
    if (digitalRead(HARD_KILL_PIN)) {
        kill_status &= ~(1 << HARD_KILL_IDX);
        kill_timers[HARD_KILL_IDX] = millis();
    }
    else {
        kill_status |= (1 << HARD_KILL_IDX);
        kill_timers[HARD_KILL_IDX] = 0;
    }
    */

    // We don't want to change light tower if only ESC kill
    bool true_kill = kill_status & ~((1 << ESC1_KILL_IDX) + (1 << ESC2_KILL_IDX));
    bool esc1_kill = kill_status & (1 << ESC1_KILL_IDX);
    bool esc2_kill = kill_status & (1 << ESC2_KILL_IDX);

    uint8_t prev_kill_flag = kill_flag;

    // Update Kill
    if (true_kill) {
        kill_flag = 0b11;
        light_state = LIGHT_KILL;
    }
    else {
        kill_flag = 0b00;
        if (control_mode == BB_CTRL_MODE_AUTO) {
            light_state = LIGHT_AUTON;
        }
        else if (control_mode == BB_CTRL_MODE_STATION_KEEP) {
            light_state = LIGHT_STATIONKEEP;
        }
        else if (control_mode == BB_CTRL_MODE_RC) {
            light_state = LIGHT_RC;
        }

        // ESC Kill
        if (esc1_kill) {
            kill_flag |= (1 << ESC1_IDX);
        }
        if (esc2_kill) {
            kill_flag |= (1 << ESC2_IDX);
        }
    }

    // Send kill commands
    bool esc1_kill_flag = kill_flag & (1 << ESC1_IDX);
    bool esc2_kill_flag = kill_flag & (1 << ESC2_IDX);

    // We cannot receive status flag from ESC while contactor is open (no power)
    id = can_id::CONTACTOR_CTRL;
    len = 2;

    if (esc1_kill_flag != (bool)(prev_kill_flag & (1 << ESC1_IDX))) {
        if (esc1_kill_flag) {
            esc1.kill();
        }
        else {
            esc1.unkill();

            buf[0] = 0;
            buf[1] = 1;
            can.sendMsgBuf(id, 0, len, buf);
        }
    }
    if (esc2_kill_flag != (bool)(prev_kill_flag & (1 << ESC2_IDX))) {
        if (esc2_kill_flag) {
            esc2.kill();
        }
        else {
            esc2.unkill();
            
            buf[0] = 0;
            buf[1] = 2;
            can.sendMsgBuf(id, 0, len, buf);
        }
    }
}


//==========================================
//
//        GLOBAL UPDATES FUNCTIONS
//
//==========================================
void update_timeout() {
    update_esc_heartbeat();
    // update_kill(); // Kill timeout, removed for now
    update_sensor_validity();
}

void update_esc_heartbeat() {
    for (int i = 0; i < ESC_COUNT; i++) {
        if (millis() - esc_heartbeat_timers[i] > ESC_HEARTBEAT_TIMEOUT) {
            esc_heartbeat[i] = false;
        }
        else {
            esc_heartbeat[i] = true;
        }
    }
}

// Currently not in use
void update_kill() {
    for (int i = 0; i < KILL_TYPE_COUNT; i++) {
        if (!(kill_status & (1 << i)) && millis() - kill_timers[i] > KILL_TIMEOUT) {
            kill_status |= (1 << i);
            kill_timers[i] = 0;
        }
    }
}

void update_sensor_validity() {
    for (int i = 0; i < SENSOR_TYPE_COUNT; i++) {
        if (millis() - sensor_timers[i] > SENSOR_TIMEOUT) {
            sensors_validity &= ~(1 << i);
        }
        else {
            sensors_validity |= (1 << i);
        }
    }
}
