#ifndef _DEFINES_H
#define _DEFINES_H

/*CAN Init*/
#define CAN_CS 11

/*CAN Tx/Rx*/
#define NUM_CAN_TIMER_ID 7
#define HEARTBEAT_TIMER_IDX 0
#define HARDKILL_TIMER_IDX 1
#define BATT1_STATS_TIMER_IDX 2
#define BATT2_STATS_TIMER_IDX 3
#define PMIC_REST_STATS_TIMER_IDX 4
#define PMIC_PC_STATS_TIMER_IDX 5
#define PH_STATS_TIMER_IDX 6

// CAN Tx Loops
#define CAN_HEARTBEAT_LOOP 500
#define CAN_HARDKILL_LOOP 500
#define CAN_BATT1_STATS_LOOP 250
#define CAN_BATT2_STATS_LOOP 250
#define CAN_PMIC_REST_STATS_LOOP 500
#define CAN_PMIC_PC_STATS_LOOP 500
#define CAN_PH_STATS_LOOP 1000

/*Battery monitoring*/
#define NUM_BATT 2
#define BATT1_IDX 0
#define BATT2_IDX 1

// Battery status/data polling timeouts
#define BATT_UPDATE_TIMEOUT 333
#define BATT_IDLE_TIMEOUT 2000
#define BATT_CONTROL_TIMEOUT 8000

/*SHT4x*/
#define SHT_LOOP 10
#define SHT_TEMP_IDX 0
#define SHT_HUMIDITY_IDX 1
#define STAT_COUNT 2

/*Temperature*/
// #define TEMP_PIN A4

#define ISO_OFF_PC A9
#define ISO_OFF_REST A10

#define ONBOARD_KILL 23
#define PEN_1 24
// #define ALERT_REST 29
// #define ALERT_PC 35

// #define TX_RS485_1 16
// #define RX_RS485_1 17
// #define RD_RS485_1
// #define IN_RS485_1 7
// #define OUT_RS485_1 6

// #define TX_RS485_2 14
// #define RX_RS485_2 15
// #define RD_RS485_2 
// #define IN_RS485_2
// #define OUT_RS485_2

// #define BATT_ON_1 38
// #define BATT_ON_2 

#endif // _DEFINES_H