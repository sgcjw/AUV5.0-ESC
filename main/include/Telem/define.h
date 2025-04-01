#ifndef _DEFINES_H
#define _DEFINES_H

// CAN
#define CAN_CS  11 // PB5
#define CAN_INT 2  // PE4

// SCREEN
#define SCREEN_INT   12 // PB6
#define SCREEN_CS    53 // PB0
#define SCREEN_RESET 5  // PE3
#define OFFSET       25

#define SCREEN_LOOP    1000
#define HEARTBEAT_LOOP 500
#define HB_TIMEOUT     2000
#define STATS_TIMEOUT  3000

// screen colours
#define RA8875_BLACK   0x0000 ///< Black Color
#define RA8875_BLUE    0x001F ///< Blue Color
#define RA8875_RED     0xF800 ///< Red Color
#define RA8875_GREEN   0x07E0 ///< Green Color
#define RA8875_CYAN    0x07FF ///< Cyan Color
#define RA8875_MAGENTA 0xF81F ///< Magenta Color
#define RA8875_YELLOW  0xFFE0 ///< Yellow Color
#define RA8875_ORANGE  0xFA00 ///< Orange Color
#define RA8875_WHITE   0xFFFF ///< White Color

// LEDs
// Physical placement in PCB is 4 3 2 1
#define RSSI_LED1    A10
#define RSSI_LED2    A9
#define RSSI_LED3    A8
#define RSSI_LED4    A7
#define GENERAL_LED1 A11
#define GENERAL_LED2 A12

// RC
/* RF link stuff */
// followed this https://www.metageek.com/training/resources/understanding-rssi/
#define RSSI_BEST    (int8_t)(-30)
#define RSSI_GOOD    (int8_t)(-67)
#define RSSI_OKAY    (int8_t)(-70)
#define RSSI_NOTGOOD (int8_t)(-80)

// followed this https://oscarliang.com/lq-rssi/
// -105 dBm is ELRS sensitivity limit for 333Hz packet rate, + 10 for safety margin
#define MIN_RSSI (int8_t)(-105 + 10)

// Telem loops
#define RCTELEMLOPP 1000

/* Channels 1 - 16 */
#define YAW_PAN_CH       1
#define SURGE_TILT_CH    2
#define SWAY_CH          3
#define FLYWHEEL_CH      4
#define KILL_CH          5
#define VEHICLE_MODE_CH  6
#define THRUST_DEPLOY_CH 7
#define HYDRO_DEPLOY_CH  8
#define ACT_LOCK_CH      9
#define LOAD_SHOOT_CH    10
#define ACT_CAL_CH       11
#define MHPB_CYCLE_CH    12
#define THRUST_AZI_CH    13
#define GIMBAL_MODE_CH   14
// #define _CH 15
// #define _CH 16

/* Channels Thresholds */
// sticks, knobs and slider
#define CHANNEL_DEADZONE 10

#define CHANNEL_MIN_VAL               CRSF_CHANNEL_MIN
#define CHANNEL_MIN_DEADZONE_TH       CRSF_CHANNEL_MIN + CHANNEL_DEADZONE
#define CHANNEL_MID_DEADZONE_LOWER_TH CHANNEL_MID_VAL - CHANNEL_DEADZONE
#define CHANNEL_MID_VAL               CRSF_CHANNEL_MID
#define CHANNEL_MID_DEADZONE_UPPER_TH CHANNEL_MID_VAL + CHANNEL_DEADZONE
#define CHANNEL_MAX_VAL               CRSF_CHANNEL_MAX

// 3 pos switch
// min      TRISTATE_LOW_TH     mid     TRISTATE_HIGH_TH    max
// State 0                      State 1                     State 2
#define TRISTATE_HIGH_TH 1500
#define TRISTATE_LOW_TH  500

// 2 pos switch
// min      BISTATE_TH      max
// State 0                  State 1
#define BISTATE_TH 900

// ACT cal control
#define CAL_PB_AZI_RC_VAL  272
#define CAL_PQ_AZI_RC_VAL  352
#define CAL_SB_AZI_RC_VAL  432
#define CAL_SQ_AZI_RC_VAL  512
#define CAL_PAN_RC_VAL     592
#define CAL_TILT_RC_VAL    672
#define CAL_PB_VERT_RC_VAL 752
#define CAL_PQ_VERT_RC_VAL 832
#define CAL_SB_VERT_RC_VAL 912
#define CAL_SQ_VERT_RC_VAL 992
#define CAL_HY_VERT_RC_VAL 1072
#define CAL_IDLE_RC_VAL    1792

// MHPB cycle control
#define MHPB_PSU_RC_VAL     272
#define MHPB_NAV_RC_VAL     352
#define MHPB_EXT_24V_RC_VAL 432
#define MHPB_OUST_RC_VAL    512
#define MHPB_EXT_5V_RC_VAL  592
#define MHPB_FANS_RC_VAL    672
#define MHPB_LBKP_RC_VAL    752
#define MHPB_ETH_RC_VAL     832
#define MHPB_IDLE_RC_VAL    1792

// thusters Azi control
#define TH_AZI_0_RC_VAL 172
#define TH_AZI_1_RC_VAL 500
#define TH_AZI_2_RC_VAL 828
#define TH_AZI_3_RC_VAL 1156
#define TH_AZI_4_RC_VAL 1484
#define TH_AZI_5_RC_VAL 1810

// other nonsense
#define CAL_HOLD_MS 3000

/* Thrusters vroom vroom*/
constexpr uint16_t THRUSTER_AZI_NEG_45_DEG = 16384; // maps to 45.0014 ↖
constexpr uint16_t THRUSTER_AZI_NEG_30_DEG = 21846; // maps to 29.9991 ↖
constexpr uint16_t THRUSTER_AZI_0_DEG = 32768;      // maps to 0 ↑
constexpr uint16_t THRUSTER_AZI_POS_30_DEG = 43690; // maps to 29.9991 ↗
constexpr uint16_t THRUSTER_AZI_POS_45_DEG = 49152; // maps to 45.0014 ➚

/* Ballshooter pew pew */
constexpr uint16_t BALLSHOOTER_INC_MAX_VAL = 50;
constexpr uint16_t BALLSHOOTER_0_DEG = 32768; // maps to 0 ↑

/* RC Telemetry  */
// The maximum values for the telemetry data formatted for sending, used to prevent sending overflowed data
#define RC_TELEM_BATT_V_MAX     300
#define RC_TELEM_BATT_V_TIMEOUT 511

#define RC_TELEM_BATT_I_MAX     20000
#define RC_TELEM_BATT_I_TIMEOUT 21000

#define RC_TELEM_PC_V_MAX     253
#define RC_TELEM_PC_V_TIMEOUT 255

#define RC_TELEM_REST_V_MAX     253
#define RC_TELEM_REST_V_TIMEOUT 255

#define RC_TELEM_PC_I_MAX     20000
#define RC_TELEM_PC_I_TIMEOUT 21000

#define RC_TELEM_REST_I_MAX     253
#define RC_TELEM_REST_I_TIMEOUT 255

#define RC_TELEM_TEMP_MAX     125
#define RC_TELEM_TEMP_TIMEOUT 127

#define RC_TELEM_HUM_MAX     125
#define RC_TELEM_HUM_TIMEOUT 127

#define RC_TELEM_TILTPAN_ERROR       95
#define RC_TELEM_TILTPAN_CALIBRATING 96
#define RC_TELEM_TILTPAN_STATUS2     97
#define RC_TELEM_TILTPAN_STATUS3     98
#define RC_TELEM_TILTPAN_STATUS4     99
#define RC_TELEM_TILTPAN_TIMEOUT     100

#define RC_TELEM_FLYWHEEL_ERROR   250
#define RC_TELEM_FLYWHEEL_STATUS1 251
#define RC_TELEM_FLYWHEEL_STATUS2 252
#define RC_TELEM_FLYWHEEL_STATUS3 253
#define RC_TELEM_FLYWHEEL_STATUS4 254
#define RC_TELEM_FLYWHEEL_TIMEOUT 255

#define RC_TELEM_LOADER_TIMEOUT 3

#define RC_TELEM_VERT_ACT_TIMEOUT 255

#define RC_TELEM_AZI_ERROR       250
#define RC_TELEM_AZI_CALIBRATING 251
#define RC_TELEM_AZI_STATUS2     252
#define RC_TELEM_AZI_STATUS3     253
#define RC_TELEM_AZI_STATUS4     254
#define RC_TELEM_AZI_TIMEOUT     255

#define RC_TELEM_TOTAL_BYTES 42

/* CAN */
#define CAN_HB_LOOP                  500  // 2 Hz
#define CAN_KILL_LOOP                500  // 10 Hz
#define CAN_CTRL_LINK_LOOP           500  // 2 Hz
#define CAN_MHPB_LOOP                1000 // 1 Hz
#define CAN_THRUSTER_LOOP            50   // 20 Hz
#define CAN_CALIBRATION_LOOP         1000 // 1 Hz
#define CAN_THRUST_HYDRO_DEPLOY_LOOP 1000 // 1 Hz
#define CAN_THRUST_AZI_LOOP          50   // 20 Hz
#define CAN_BALL_SHOOTER_LOOP        50   // 20 Hz

#endif // _DEFINES_H