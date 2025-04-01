#ifndef _DEFINES_H
#define _DEFINES_H

//===========================================
//
//        CAN INIT
//
//===========================================
#define CAN_CHIP_SELECT 10
#define CAN_INT 2


//===========================================
//
//        THRUSTERS
//
//===========================================
// Thruster CAN IDs
#define ROBOTEQ_CAN1_SEND_INDEX 0x605
#define ROBOTEQ_CAN1_REPLY_INDEX 0x585
#define ROBOTEQ_CAN2_SEND_INDEX 0x606
#define ROBOTEQ_CAN2_REPLY_INDEX 0x586

#define THRUSTER_COUNT 4
#define ESC_COUNT 2
#define ESC1_IDX 0
#define ESC2_IDX 1
// #define ESC1_ID 0x601
// #define ESC2_ID 0x604
#define ESC_HEARTBEAT_TIMEOUT 1000
#define ESC_CMD_TIMEOUT 500

#define E_STOP_BIT 4


//===========================================
//
//        LIGHT TOWER
//
//===========================================
// Light Tower Pins
#define LIGHT_GATE_COUNT 3
#define GREEN_GATE 5
#define YELLOW_GATE 6
#define RED_GATE 7

// Light Tower
#define LIGHT_AUTON 0
#define LIGHT_RC 1
#define LIGHT_KILL 2
#define LIGHT_STATIONKEEP 3

#define LIGHT_BLINK_TIME 500        // 1Hz


//===========================================
//
//        SENSORS
//
//===========================================
// Hard Kill Pin
#define HARD_KILL_PIN 3

// Temp Probe Pins
#define TEMP_PROBE_COUNT 3
#define TP1_PIN 14
#define TP2_PIN 15
#define TP3_PIN 16

// Internal Stats
#define SHT_LOOP 10
#define TEMP_PROBE_LOOP 10
#define SHT_TIMER_IDX 0
#define TEMP_PROBE_TIMER_IDX 1

#define STAT_COUNT 5
#define SHT_TEMP_IDX 0
#define SHT_HUMIDITY_IDX 1
#define TEMP_PROBE_IDX_OFFSET 2
#define TEMP_PROBE_CAN_OFFSET 4

// I2C
#define TEMP_HUMIDITY_ADDR 0x44

// Sensor Read Timeouts
#define SENSOR_TYPE_COUNT 2
#define SENSOR_TIMEOUT 1000         // 1s


//===========================================
//
//        CAN READ/WRITE
//
//===========================================
/*
CAN Publish IDs
0x20    MAIN HULL STATS
0x21    HEARTBEAT
0x27    KILL STATUS
0x29    ESC1 STATS
0x2A    ESC2 STATS
0x2B    ESC1 FLAGS
0x2C    ESC2 FLAGS
2 more for ESC control (Node ID: 5 & 6)

CAN Read IDs
0x10    SOFT KILL STATUS
0x11    CONTROL LINK
0x12    RC THRUSTER CONTROL
0x13    SBC THRUSTER CONTROL
0x14    HARDWARE KILL
0x15    ESC CONTROL
*/
#define CAN_ID_COUNT 7
#define HEARTBEAT_TIMER_IDX 0
#define KILL_STATUS_TIMER_IDX 1
#define ESC1_STATS_TIMER_IDX 2
#define ESC2_STATS_TIMER_IDX 3
#define ESC1_FLAGS_TIMER_IDX 4
#define ESC2_FLAGS_TIMER_IDX 5
#define MH_STATS_TIMER_IDX 6

// CAN WRITE LOOPS
#define HEARTBEAT_LOOP 500          // publish heartbeat every 500ms
#define KILL_STATUS_LOOP 500        // publish kill status every 500ms
#define ESC_STATS_LOOP 250          // publish esc stats every 250ms
#define ESC_FLAGS_LOOP 250          // publish esc flags every 250ms
#define MH_STATS_LOOP 1000          // publish stats every 1s
#define THRUSTER_DATA_LOOP 0        // publish thruster data as fast as possible


//===========================================
//
//        KILL
//
//===========================================
#define SOFT_KILL_SOURCE_SBC 1
#define SOFT_KILL_SOURCE_RC 2

/*
ESC KILL does not change light, is for power cycle
*/
#define KILL_TYPE_COUNT 6
#define HARD_KILL_IDX 0
#define SBC_SOFT_KILL_IDX 1
#define RC_SOFT_KILL_IDX 2
#define TELEM_HB_KILL_IDX 3
#define ESC1_KILL_IDX 4
#define ESC2_KILL_IDX 5

#define KILL_TIMEOUT 1500           // 1.5s

#endif