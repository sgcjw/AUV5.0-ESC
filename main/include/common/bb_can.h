/**
 *  ========================
 *      ASV4 CAN STANDARD
 *  ========================
 *
 *  Define actual values in this header file ONLY -- this is the single source of truth
 *  - Names should follow the syntax: BB_<TYPE>_<NAME>
 *  - Best to roughly follow the naming used in the namespaced version defined in `bb_can.hpp`
 *  This header file is necessary to support C files (STM32)
 *  - Namespaced definitions in `bb_can.hpp` only work for C++ files
 * Link to Excel:
 * https://nusu.sharepoint.com/:x:/r/sites/Bumblebee/Shared%20Documents/ASV%204.0/Electrical/ASV%204.0%20CAN%20Standard.xlsx?d=w60cd9a3b825e485e893c701c3a49e49d&csf=1&web=1&e=BFI0Xj
 */

#pragma once

#include <stdint.h>

#define BB_MASK(x) (1UL << (x))

// clang-format off

/*
 * CAN IDs
 */
// Thruster Group
#define BB_CAN_ID_SOFT_KILL                 (16)
#define BB_CAN_ID_CTRL_LINK                 (17)
#define BB_CAN_ID_RC_THRUSTER               (18)
#define BB_CAN_ID_PC_THRUSTER               (19)
#define BB_CAN_ID_HARD_KILL                 (20)
#define BB_CAN_ID_ESC_CTRL                  (21)

// Telem Group
#define BB_CAN_ID_MAINHULL_STATS            (32)
#define BB_CAN_ID_HEARTBEAT                 (33)
#define BB_CAN_ID_POWERHULL_STATS           (34)
#define BB_CAN_ID_PC_TEMP                   (35)
#define BB_CAN_ID_BATT1_STATS               (36)
#define BB_CAN_ID_BATT2_STATS               (37)
#define BB_CAN_ID_BATT_FLAGS                (38)
#define BB_CAN_ID_KILL_STATUS               (39)
#define BB_CAN_ID_LOGIC_CTRL                (40)
#define BB_CAN_ID_ESC1_STATS                (41)
#define BB_CAN_ID_ESC2_STATS                (42)
#define BB_CAN_ID_ESC1_MOTOR_FLAGS          (43)
#define BB_CAN_ID_ESC2_MOTOR_FLAGS          (44)
#define BB_CAN_ID_MHPB_POWER_STATUS         (45)
#define BB_CAN_ID_LOGIC_STATUS              (46)
#define BB_CAN_ID_POPB_PC_STATS             (47)
#define BB_CAN_ID_POPB_REST_STATS           (48)
#define BB_CAN_ID_ACT_STATS                 (49)
#define BB_CAN_ID_NAV_STATS                 (50)
#define BB_CAN_ID_ACT_AZI_STATS             (51)
#define BB_CAN_ID_ACT_VERT_STATS            (52)
#define BB_CAN_ID_ACT_BS_STATS              (53)

// Actuation Group
#define BB_CAN_ID_RC_ACTUATION_VERT         (64)
#define BB_CAN_ID_RC_THRUSTER_AZI           (65)
#define BB_CAN_ID_PC_ACTUATION_VERT         (66)
#define BB_CAN_ID_PC_THRUSTER_AZI           (67)
#define BB_CAN_ID_RC_BALLSHOOTER            (69)
#define BB_CAN_ID_PC_BALLSHOOTER            (70)

// Power Control Group
#define BB_CAN_ID_PC_MHPB_POWER_CTRL        (80)
#define BB_CAN_ID_RC_MHPB_POWER_CTRL        (81)
#define BB_CAN_ID_CONTACTOR_CTRL            (87)
#define BB_CAN_ID_BATT_CTRL                 (88)

/*
 *  Heartbeat IDs
 */
#define BB_HEARTBEAT_ID_PC                  (1)
#define BB_HEARTBEAT_ID_LOGIC               (2)
#define BB_HEARTBEAT_ID_MHPB                (3)
#define BB_HEARTBEAT_ID_THRUSTERS_BOARD     (4)
#define BB_HEARTBEAT_ID_TELEM_BOARD         (5)
#define BB_HEARTBEAT_ID_RC                  (6)
#define BB_HEARTBEAT_ID_POPB                (17)
#define BB_HEARTBEAT_ID_ESC1                (33)
#define BB_HEARTBEAT_ID_ESC2                (34)
#define BB_HEARTBEAT_ID_BATT1               (49)
#define BB_HEARTBEAT_ID_BATT2               (50)
#define BB_HEARTBEAT_ID_ACTUATION           (65)
#define BB_HEARTBEAT_ID_NAV_RPI             (81)

/*
 * Frequencies definitions (period - in ms)
 */
#define BB_FREQ_SOFT_KILL                (500)
#define BB_FREQ_CTRL_LINK                (500)
#define BB_FREQ_RC_THRUSTER              (50)
#define BB_FREQ_PC_THRUSTER              (50)
#define BB_FREQ_HARD_KILL                (500)
#define BB_FREQ_ESC_CTRL                 (50)
#define BB_FREQ_MAINHULL_STATS           (1000)
#define BB_FREQ_HEARTBEAT                (500)
#define BB_FREQ_POWERHULL_STATS          (1000)
#define BB_FREQ_PC_TEMP                  (1000)
#define BB_FREQ_BATT1_STATS              (500)
#define BB_FREQ_BATT2_STATS              (500)
#define BB_FREQ_BATT_FLAGS               (500)
#define BB_FREQ_KILL_STATUS              (500)
#define BB_FREQ_LOGIC_CTRL               (1000)
#define BB_FREQ_ESC1_STATS               (500)
#define BB_FREQ_ESC2_STATS               (500)
#define BB_FREQ_ESC1_MOTOR_FLAGS         (500)
#define BB_FREQ_ESC2_MOTOR_FLAGS         (500)
#define BB_FREQ_MHPB_POWER_STATUS        (1000)
#define BB_FREQ_LOGIC_STATUS             (1000)
#define BB_FREQ_POPB_PC_STATS            (1000)
#define BB_FREQ_POPB_REST_STATS          (1000)
#define BB_FREQ_ACT_STATS                (1000)
#define BB_FREQ_NAV_STATS                (1000)
#define BB_FREQ_ACT_AZI_STATS            (1000)
#define BB_FREQ_ACT_VERT_STATS           (1000)
#define BB_FREQ_ACT_BS_STATS             (1000)
#define BB_FREQ_RC_ACTUATION_VERT        (1000)
#define BB_FREQ_RC_THRUSTER_AZI          (50)
#define BB_FREQ_PC_ACTUATION_VERT        (1000)
#define BB_FREQ_PC_THRUSTER_AZI          (50)
#define BB_FREQ_RC_BALLSHOOTER           (50)
#define BB_FREQ_PC_BALLSHOOTER           (50)
#define BB_FREQ_PC_MHPB_POWER_CTRL       (1000)
#define BB_FREQ_RC_MHPB_POWER_CTRL       (1000)
#define BB_FREQ_CONTACTOR_CTRL           (1000)
#define BB_FREQ_BATT_CTRL                (1000)

/*
 *  Control Mode
 *  - Packet data for bb::can_id::CTRL_LINK
 */
#define BB_CTRL_MODE_AUTO                   (1)
#define BB_CTRL_MODE_RC                     (2)
#define BB_CTRL_MODE_STATION_KEEP           (3)

/*
 *	MHPB (Main Hull Power Board) Status and Control Bits
 *	- Packet data for bb::can_id::MHPB_STATUS
 *	- Packet data for bb::can_id::MHPB_CTRL
 */
#define BB_MHPB_BITPOS_SWITCH         0
#define BB_MHPB_BITPOS_LOGIC          1
#define BB_MHPB_BITPOS_PUMP           2
#define BB_MHPB_BITPOS_EXTRA_5V       3
#define BB_MHPB_BITPOS_OUSTER         4
#define BB_MHPB_BITPOS_EXTRA_24V      5
#define BB_MHPB_BITPOS_NAV_HULL       6
#define BB_MHPB_BITPOS_PSU            7

/* Power Control*/
// Enable Request for Power Channels
// Request to turn off Switch, Logic, Pump(& fans) and PSU will be ignored
// 0 = request to enable, 1 = request to disable
#define BB_MHPB_POWER_CTRL_EN_BIT_SWITCH            BB_MASK(BB_MHPB_BITPOS_SWITCH)
#define BB_MHPB_POWER_CTRL_EN_BIT_LOGIC             BB_MASK(BB_MHPB_BITPOS_LOGIC)
#define BB_MHPB_POWER_CTRL_EN_BIT_PUMP              BB_MASK(BB_MHPB_BITPOS_PUMP)
#define BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_5V          BB_MASK(BB_MHPB_BITPOS_EXTRA_5V)
#define BB_MHPB_POWER_CTRL_EN_BIT_OUSTER            BB_MASK(BB_MHPB_BITPOS_OUSTER)
#define BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_24V         BB_MASK(BB_MHPB_BITPOS_EXTRA_24V)
#define BB_MHPB_POWER_CTRL_EN_BIT_NAV_HULL          BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_POWER_CTRL_EN_BIT_PSU               BB_MASK(BB_MHPB_BITPOS_PSU)

// Power Cycle Request for Power Channels
// 0 = idle, 1 = request to cycle
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_SWITCH         BB_MASK(BB_MHPB_BITPOS_SWITCH)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_LOGIC          BB_MASK(BB_MHPB_BITPOS_LOGIC)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_PUMP           BB_MASK(BB_MHPB_BITPOS_PUMP)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_5V       BB_MASK(BB_MHPB_BITPOS_EXTRA_5V)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_OUSTER         BB_MASK(BB_MHPB_BITPOS_OUSTER)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_24V      BB_MASK(BB_MHPB_BITPOS_EXTRA_24V)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_NAV_HULL       BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_POWER_CTRL_CYCLE_BIT_PSU            BB_MASK(BB_MHPB_BITPOS_PSU)

// Shutdown Request for PC and NAV
// 0 = idle, 1 = allow for shutdown
#define BB_MHPB_SHUTDOWN_REQUEST_BIT_NAV_HULL       BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_SHUTDOWN_REQUEST_BIT_PC             BB_MASK(BB_MHPB_BITPOS_PSU)          // PSU is used for PC

/* Power Status*/
// Enable status of the channels
// 0 = channel is enabled, 1 = channel is disabled
#define BB_MHPB_POWER_STATS_EN_BIT_SWITCH         BB_MASK(BB_MHPB_BITPOS_SWITCH)
#define BB_MHPB_POWER_STATS_EN_BIT_LOGIC          BB_MASK(BB_MHPB_BITPOS_LOGIC)
#define BB_MHPB_POWER_STATS_EN_BIT_PUMP           BB_MASK(BB_MHPB_BITPOS_PUMP)
#define BB_MHPB_POWER_STATS_EN_BIT_EXTRA_5V       BB_MASK(BB_MHPB_BITPOS_EXTRA_5V)
#define BB_MHPB_POWER_STATS_EN_BIT_OUSTER         BB_MASK(BB_MHPB_BITPOS_OUSTER)
#define BB_MHPB_POWER_STATS_EN_BIT_EXTRA_24V      BB_MASK(BB_MHPB_BITPOS_EXTRA_24V)
#define BB_MHPB_POWER_STATS_EN_BIT_NAV_HULL       BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_POWER_STATS_EN_BIT_PSU            BB_MASK(BB_MHPB_BITPOS_PSU)

// Cycle status of the channels (power cycle can be induced from request or attempt to fault recover)
// 0 = idle, 1 = power cycling
#define BB_MHPB_POWER_STATS_CYCLE_BIT_SWITCH         BB_MASK(BB_MHPB_BITPOS_SWITCH)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_LOGIC          BB_MASK(BB_MHPB_BITPOS_LOGIC)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_PUMP           BB_MASK(BB_MHPB_BITPOS_PUMP)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_EXTRA_5V       BB_MASK(BB_MHPB_BITPOS_EXTRA_5V)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_OUSTER         BB_MASK(BB_MHPB_BITPOS_OUSTER)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_EXTRA_24V      BB_MASK(BB_MHPB_BITPOS_EXTRA_24V)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_NAV_HULL       BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_POWER_STATS_CYCLE_BIT_PSU            BB_MASK(BB_MHPB_BITPOS_PSU)

// Fault status of the channels
// 0 = good, 1 = bad
#define BB_MHPB_POWER_STATS_FAULT_BIT_SWITCH         BB_MASK(BB_MHPB_BITPOS_SWITCH)
#define BB_MHPB_POWER_STATS_FAULT_BIT_LOGIC          BB_MASK(BB_MHPB_BITPOS_LOGIC)
#define BB_MHPB_POWER_STATS_FAULT_BIT_PUMP           BB_MASK(BB_MHPB_BITPOS_PUMP)
#define BB_MHPB_POWER_STATS_FAULT_BIT_EXTRA_5V       BB_MASK(BB_MHPB_BITPOS_EXTRA_5V)
#define BB_MHPB_POWER_STATS_FAULT_BIT_OUSTER         BB_MASK(BB_MHPB_BITPOS_OUSTER)
#define BB_MHPB_POWER_STATS_FAULT_BIT_EXTRA_24V      BB_MASK(BB_MHPB_BITPOS_EXTRA_24V)
#define BB_MHPB_POWER_STATS_FAULT_BIT_NAV_HULL       BB_MASK(BB_MHPB_BITPOS_NAV_HULL)
#define BB_MHPB_POWER_STATS_FAULT_BIT_PSU            BB_MASK(BB_MHPB_BITPOS_PSU)

/*
 *  Logic Board Power Status/Control Bits
 *  - Packet data for bb:can_id::LOGIC_CTRL & bb:can_id::LOGIC_STATUS
 */
#define BB_LOGIC_POWER_BITPOS_TELEM_BOARD       1
#define BB_LOGIC_POWER_BITPOS_THRUSTERS_BOARD   2

#define BB_LOGIC_POWER_BIT_TELEM_BOARD          BB_MASK(BB_LOGIC_POWER_BITPOS_TELEM_BOARD)
#define BB_LOGIC_POWER_BIT_THRUSTERS_BOARD      BB_MASK(BB_LOGIC_POWER_BITPOS_THRUSTERS_BOARD)

/*
 *  Kill Bits
 */
#define BB_KILL_BITPOS_HARD                 0
#define BB_KILL_BITPOS_PC                   1
#define BB_KILL_BITPOS_RC                   2
#define BB_KILL_BITPOS_TELEM_NO_HB          3
#define BB_KILL_BITPOS_ESC1                 4
#define BB_KILL_BITPOS_ESC2                 5

#define BB_KILL_BIT_HARD                    BB_MASK(BB_KILL_BITPOS_HARD)
#define BB_KILL_BIT_PC                      BB_MASK(BB_KILL_BITPOS_PC)
#define BB_KILL_BIT_RC                      BB_MASK(BB_KILL_BITPOS_RC)
#define BB_KILL_BIT_TELEM_NO_HB             BB_MASK(BB_KILL_BITPOS_TELEM_NO_HB)
#define BB_KILL_BIT_ESC1                    BB_MASK(BB_KILL_BITPOS_ESC1)
#define BB_KILL_BIT_ESC2                    BB_MASK(BB_KILL_BITPOS_ESC2)

/*
 * Thruster Control
 * - Packet data for bb::can_id::RC_THRUSTER & bb::can_id::PC_THRUSTER
 */
#define BB_THRUSTER_REVERSE_MAX              0
#define BB_THRUSTER_STOP                     3200
#define BB_THRUSTER_FORWARD_MAX              6400

/*
 *  Act Azimuth Stats
 *  - Packet data for bb::can_id::ACT_AZI_STATUS   
*/
#define BB_ACT_AZI_STATS_ERROR             0
#define BB_ACT_AZI_STATS_CALIBRATING       1
#define BB_ACT_AZI_STATS_RIGHT             5
#define BB_ACT_AZI_STATS_LEFT              65535

/*
 *  Act Vertical Stats
 *  - Packet data for bb::can_id::ACT_VERT_STATUS
 */
#define BB_ACT_VERT_STATS_ERROR            0
#define BB_ACT_VERT_STATS_CALIBRATING      1
#define BB_ACT_VERT_STATS_LOWEST           5
#define BB_ACT_VERT_STATS_HIGHEST          255

/*
 *  BS Stats (Telem)
 *  - Packet data for bb::can_id::ACT_BS_STATUS
 */
#define BB_BS_STATS_BALL_LOADER_ERROR            0
#define BB_BS_STATS_BALL_LOADER_UNLOADED         1
#define BB_BS_STATS_BALL_LOADER_LOADED           2
#define BB_BS_STATS_TILT_ERROR                   0
#define BB_BS_STATS_TILT_CALIBRATING             1
#define BB_BS_STATS_TILT_DOWN                    5
#define BB_BS_STATS_TILT_UP                      65535
#define BB_BS_STATS_PAN_ERROR                    0
#define BB_BS_STATS_PAN_CALIBRATING              1
#define BB_BS_STATS_PAN_LEFT                     5
#define BB_BS_STATS_PAN_RIGHT                    65535
#define BB_BS_STATS_FLYWHEEL_ERROR               0
#define BB_BS_STATS_FLYWHEEL_OFF                 5
#define BB_BS_STATS_FLYWHEEL_FORWARD_MAX         65535

/*
 *  RC Vertical Act Control
 *  - Packet data for bb::can_id::RC_ACTUATION_VERT
 */
#define BB_RC_ACTUATION_VERT_IGNORE               0
#define BB_RC_ACTUATION_VERT_CALIBRATE            1
#define BB_RC_ACTUATION_VERT_LOWEST               5
#define BB_RC_ACTUATION_VERT_HIGHEST              255

/* 
 * RC Thruster Azimuth Control
 * - Packet data for bb::can_id::RC_THRUSTER_AZI
 */
#define BB_RC_THRUSTER_AZI_IGNORE                 0
#define BB_RC_THRUSTER_AZI_CALIBRATE              1
#define BB_RC_THRUSTER_AZI_RIGHT                  5
#define BB_RC_THRUSTER_AZI_LEFT                   65535

/*
 *  PC Vertical Act Control
 *  - Packet data for bb::can_id::PC_ACTUATION_VERT
 */
#define BB_PC_ACTUATION_VERT_IGNORE              0
#define BB_PC_ACTUATION_VERT_CALIBRATE           1
#define BB_PC_ACTUATION_VERT_LOWEST              5
#define BB_PC_ACTUATION_VERT_HIGHEST             255

/*
 *  PC Thruster Azimuth Control
 *  - Packet data for bb::can_id::PC_THRUSTER_AZI
 */
#define BB_PC_THRUSTER_AZI_IGNORE                0
#define BB_PC_THRUSTER_AZI_CALIBRATE             1
#define BB_PC_THRUSTER_AZI_RIGHT                 5
#define BB_PC_THRUSTER_AZI_LEFT                  65535

// /*
//  * Act Calibration Bits
//  * - Packet data for bb::can_id::CALIBRATION
//  * - Request to calibrate the which actuator
//  * - Origin of the calibration
//  */
// #define BB_ACT_CALIBRATION_ORIGIN_PC                  1
// #define BB_ACT_CALIBRATION_ORIGIN_RC                  2
// #define BB_ACT_CALIBRATION_REQUEST_IGNORE             0
// #define BB_ACT_CALIBRATION_REQUEST_THRUSTERS          1
// #define BB_ACT_CALIBRATION_REQUEST_HYDROPHONES        2
// #define BB_ACT_CALIBRATION_REQUEST_BALLSHOOTER        3

/*
 *  RC BS Control
 *  - Packet data for bb::can_id::RC_BALLSHOOTER
 */
#define BB_RC_BALLSHOOTER_LOADER_IGNORE          0
#define BB_RC_BALLSHOOTER_LOADER_LOAD            1
#define BB_RC_BALLSHOOTER_LOADER_SHOOT           2
#define BB_RC_BALLSHOOTER_TILT_IGNORE            0
#define BB_RC_BALLSHOOTER_TILT_CALIBRATE         1
#define BB_RC_BALLSHOOTER_TILT_DOWN              5
#define BB_RC_BALLSHOOTER_TILT_UP                65535
#define BB_RC_BALLSHOOTER_PAN_IGNORE             0
#define BB_RC_BALLSHOOTER_PAN_CALIBRATE          1
#define BB_RC_BALLSHOOTER_PAN_LEFT               5
#define BB_RC_BALLSHOOTER_PAN_RIGHT              65535
#define BB_RC_BALLSHOOTER_FLYWHEEL_IGNORE        0
#define BB_RC_BALLSHOOTER_FLYWHEEL_OFF           5
#define BB_RC_BALLSHOOTER_FLYWHEEL_FORWARD_MAX   65535

/*
 *  PC BS Control
 *  - Packet data for bb::can_id::PC_BALLSHOOTER
 */
#define BB_PC_BALLSHOOTER_LOADER_IGNORE            0
#define BB_PC_BALLSHOOTER_LOADER_LOAD              1
#define BB_PC_BALLSHOOTER_LOADER_SHOOT             2
#define BB_PC_BALLSHOOTER_TILT_IGNORE              0
#define BB_PC_BALLSHOOTER_TILT_CALIBRATE           1
#define BB_PC_BALLSHOOTER_TILT_DOWN                5
#define BB_PC_BALLSHOOTER_TILT_UP                  65535
#define BB_PC_BALLSHOOTER_PAN_IGNORE               0
#define BB_PC_BALLSHOOTER_PAN_CALIBRATE            1
#define BB_PC_BALLSHOOTER_PAN_LEFT                 5
#define BB_PC_BALLSHOOTER_PAN_RIGHT                65535
#define BB_PC_BALLSHOOTER_FLYWHEEL_IGNORE          0
#define BB_PC_BALLSHOOTER_FLYWHEEL_OFF             5
#define BB_PC_BALLSHOOTER_FLYWHEEL_FORWARD_MAX     65535

/*
 * CAN Masks and Filters
 */
#define BB_CAN_MASK_THRUSTERS               (2032)
#define BB_CAN_FILTER_THRUSTERS             (16)

#define BB_CAN_MASK_TELEMETRY               (2016)
#define BB_CAN_FILTER_TELEMETRY             (32)

#define BB_CAN_MASK_ACTUATION               (2032)
#define BB_CAN_FILTER_ACTUATION             (64)

#define BB_CAN_MASK_POWER_CONTROL           (2032)
#define BB_CAN_FILTER_POWER_CONTROL         (80)

#define BB_CAN_MASK_ESC                     (2032)
#define BB_CAN_FILTER_ESC                   (1408)

// clang-format on

// Only include namespaced definitions for .cpp files
#ifdef __cplusplus
#include "bb_can.hpp"
#endif // __cplusplus