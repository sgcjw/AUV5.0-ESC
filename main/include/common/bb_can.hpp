/**
 *  ========================
 *      ASV4 CAN STANDARD
 *  ========================
 *
 *  Define actual values in `bb_can.hpp`
 *  - Namespaced values defined in this header should point to whatever is defined in `bb_can.h`
 * 	- DO NOT DEFINE ACTUAL VALUES HERE
 * 	C++ compatible source files should use namespaced definitions instead of macros
 */

#pragma once

#include <stdint.h>

namespace bb
{

/*
 * CAN IDs are 11-bit (or 29-bit for extended frames)
 */
using can_id_t = uint32_t;

namespace can_id
{
	constexpr can_id_t SOFT_KILL = BB_CAN_ID_SOFT_KILL;
	constexpr can_id_t CTRL_LINK = BB_CAN_ID_CTRL_LINK;
	constexpr can_id_t RC_THRUSTER = BB_CAN_ID_RC_THRUSTER;
	constexpr can_id_t PC_THRUSTER = BB_CAN_ID_PC_THRUSTER;
	constexpr can_id_t HARD_KILL = BB_CAN_ID_HARD_KILL;
	constexpr can_id_t ESC_CTRL = BB_CAN_ID_ESC_CTRL;

	constexpr can_id_t MAINHULL_STATS = BB_CAN_ID_MAINHULL_STATS;
	constexpr can_id_t HEARTBEAT = BB_CAN_ID_HEARTBEAT;
	constexpr can_id_t POWERHULL_STATS = BB_CAN_ID_POWERHULL_STATS;
	constexpr can_id_t PC_TEMP = BB_CAN_ID_PC_TEMP;
	constexpr can_id_t BATT1_STATS = BB_CAN_ID_BATT1_STATS;
	constexpr can_id_t BATT2_STATS = BB_CAN_ID_BATT2_STATS;
	constexpr can_id_t BATT_FLAGS = BB_CAN_ID_BATT_FLAGS;
	constexpr can_id_t KILL_STATUS = BB_CAN_ID_KILL_STATUS;
	constexpr can_id_t LOGIC_CTRL = BB_CAN_ID_LOGIC_CTRL;
	constexpr can_id_t ESC1_STATS = BB_CAN_ID_ESC1_STATS;
	constexpr can_id_t ESC2_STATS = BB_CAN_ID_ESC2_STATS;
	constexpr can_id_t ESC1_MOTOR_FLAGS = BB_CAN_ID_ESC1_MOTOR_FLAGS;
	constexpr can_id_t ESC2_MOTOR_FLAGS = BB_CAN_ID_ESC2_MOTOR_FLAGS;
	constexpr can_id_t MHPB_POWER_STATUS = BB_CAN_ID_MHPB_POWER_STATUS;
	constexpr can_id_t LOGIC_STATUS = BB_CAN_ID_LOGIC_STATUS;
	constexpr can_id_t POPB_PC_STATS = BB_CAN_ID_POPB_PC_STATS;
	constexpr can_id_t POPB_REST_STATS = BB_CAN_ID_POPB_REST_STATS;
	constexpr can_id_t ACT_STATS = BB_CAN_ID_ACT_STATS;
	constexpr can_id_t NAV_STATS = BB_CAN_ID_NAV_STATS;
	constexpr can_id_t ACT_AZI_STATS = BB_CAN_ID_ACT_AZI_STATS;
	constexpr can_id_t ACT_VERT_STATS = BB_CAN_ID_ACT_VERT_STATS;
	constexpr can_id_t ACT_BS_STATS = BB_CAN_ID_ACT_BS_STATS;

	constexpr can_id_t RC_ACTUATION_VERT = BB_CAN_ID_RC_ACTUATION_VERT;
	constexpr can_id_t RC_THRUSTER_AZI = BB_CAN_ID_RC_THRUSTER_AZI;
	constexpr can_id_t PC_ACTUATION_VERT = BB_CAN_ID_PC_ACTUATION_VERT;
	constexpr can_id_t PC_THRUSTER_AZI = BB_CAN_ID_PC_THRUSTER_AZI;
	constexpr can_id_t RC_BALLSHOOTER = BB_CAN_ID_RC_BALLSHOOTER;
	constexpr can_id_t PC_BALLSHOOTER = BB_CAN_ID_PC_BALLSHOOTER;

	constexpr can_id_t PC_MHPB_POWER_CTRL = BB_CAN_ID_PC_MHPB_POWER_CTRL;
	constexpr can_id_t RC_MHPB_POWER_CTRL = BB_CAN_ID_RC_MHPB_POWER_CTRL;
	constexpr can_id_t CONTACTOR_CTRL = BB_CAN_ID_CONTACTOR_CTRL;
	constexpr can_id_t BATT_CTRL = BB_CAN_ID_BATT_CTRL;
} // can_id

namespace can_mask
{
	struct can_mask_t
	{
		uint16_t mask;
		uint16_t filter;
	};

	constexpr can_mask_t ACTUATION = { .mask = BB_CAN_MASK_ACTUATION, .filter = BB_CAN_FILTER_ACTUATION };
	constexpr can_mask_t ESC = { .mask = BB_CAN_MASK_ESC, .filter = BB_CAN_FILTER_ESC };
	constexpr can_mask_t POWER_CONTROL = { .mask = BB_CAN_MASK_POWER_CONTROL, .filter = BB_CAN_FILTER_POWER_CONTROL };
	constexpr can_mask_t TELEMETRY = { .mask = BB_CAN_MASK_TELEMETRY, .filter = BB_CAN_FILTER_TELEMETRY };
	constexpr can_mask_t THRUSTERS = { .mask = BB_CAN_MASK_THRUSTERS, .filter = BB_CAN_FILTER_THRUSTERS };

} // can_mask

/*
 *	Heartbeat IDs
 * 	- Packet data for bb::can_id::HEARTBEAT
 */
namespace heartbeat_id
{
	constexpr uint8_t PC = BB_HEARTBEAT_ID_PC;
	constexpr uint8_t LOGIC = BB_HEARTBEAT_ID_LOGIC;
	constexpr uint8_t MHPB = BB_HEARTBEAT_ID_MHPB;
	constexpr uint8_t THRUSTERS_BOARD = BB_HEARTBEAT_ID_THRUSTERS_BOARD;
	constexpr uint8_t TELEM_BOARD = BB_HEARTBEAT_ID_TELEM_BOARD;
	constexpr uint8_t RC = BB_HEARTBEAT_ID_RC;
	constexpr uint8_t POPB = BB_HEARTBEAT_ID_POPB;
	constexpr uint8_t ESC1 = BB_HEARTBEAT_ID_ESC1;
	constexpr uint8_t ESC2 = BB_HEARTBEAT_ID_ESC2;
	constexpr uint8_t BATT1 = BB_HEARTBEAT_ID_BATT1;
	constexpr uint8_t BATT2 = BB_HEARTBEAT_ID_BATT2;
	constexpr uint8_t ACTUATION = BB_HEARTBEAT_ID_ACTUATION;
	constexpr uint8_t NAV_RPI = BB_HEARTBEAT_ID_NAV_RPI;
} // heartbeat_id

namespace can_freq
{
	constexpr uint16_t SOFT_KILL = BB_FREQ_SOFT_KILL;
	constexpr uint16_t CTRL_LINK = BB_FREQ_CTRL_LINK;
	constexpr uint16_t RC_THRUSTER = BB_FREQ_RC_THRUSTER;
	constexpr uint16_t PC_THRUSTER = BB_FREQ_PC_THRUSTER;
	constexpr uint16_t HARD_KILL = BB_FREQ_HARD_KILL;
	constexpr uint16_t ESC_CTRL = BB_FREQ_ESC_CTRL;
	constexpr uint16_t MAINHULL_STATS = BB_FREQ_MAINHULL_STATS;
	constexpr uint16_t HEARTBEAT = BB_FREQ_HEARTBEAT;
	constexpr uint16_t POWERHULL_STATS = BB_FREQ_POWERHULL_STATS;
	constexpr uint16_t PC_TEMP = BB_FREQ_PC_TEMP;
	constexpr uint16_t BATT1_STATS = BB_FREQ_BATT1_STATS;
	constexpr uint16_t BATT2_STATS = BB_FREQ_BATT2_STATS;
	constexpr uint16_t BATT_FLAGS = BB_FREQ_BATT_FLAGS;
	constexpr uint16_t KILL_STATUS = BB_FREQ_KILL_STATUS;
	constexpr uint16_t LOGIC_CTRL = BB_FREQ_LOGIC_CTRL;
	constexpr uint16_t ESC1_STATS = BB_FREQ_ESC1_STATS;
	constexpr uint16_t ESC2_STATS = BB_FREQ_ESC2_STATS;
	constexpr uint16_t ESC1_MOTOR_FLAGS = BB_FREQ_ESC1_MOTOR_FLAGS;
	constexpr uint16_t ESC2_MOTOR_FLAGS = BB_FREQ_ESC2_MOTOR_FLAGS;
	constexpr uint16_t MHPB_POWER_STATUS = BB_FREQ_MHPB_POWER_STATUS;
	constexpr uint16_t LOGIC_STATUS = BB_FREQ_LOGIC_STATUS;
	constexpr uint16_t POPB_PC_STATS = BB_FREQ_POPB_PC_STATS;
	constexpr uint16_t POPB_REST_STATS = BB_FREQ_POPB_REST_STATS;
	constexpr uint16_t ACT_STATS = BB_FREQ_ACT_STATS;
	constexpr uint16_t NAV_STATS = BB_FREQ_NAV_STATS;
	constexpr uint16_t ACT_AZI_STATS = BB_FREQ_ACT_AZI_STATS;
	constexpr uint16_t ACT_VERT_STATS = BB_FREQ_ACT_VERT_STATS;
	constexpr uint16_t ACT_BS_STATS = BB_FREQ_ACT_BS_STATS;
	constexpr uint16_t RC_ACTUATION_VERT = BB_FREQ_RC_ACTUATION_VERT;
	constexpr uint16_t RC_THRUSTER_AZI = BB_FREQ_RC_THRUSTER_AZI;
	constexpr uint16_t PC_ACTUATION_VERT = BB_FREQ_PC_ACTUATION_VERT;
	constexpr uint16_t PC_THRUSTER_AZI = BB_FREQ_PC_THRUSTER_AZI;
	constexpr uint16_t RC_BALLSHOOTER = BB_FREQ_RC_BALLSHOOTER;
	constexpr uint16_t PC_BALLSHOOTER = BB_FREQ_PC_BALLSHOOTER;
	constexpr uint16_t PC_MHPB_POWER_CTRL = BB_FREQ_PC_MHPB_POWER_CTRL;
	constexpr uint16_t RC_MHPB_POWER_CTRL = BB_FREQ_RC_MHPB_POWER_CTRL;
	constexpr uint16_t CONTACTOR_CTRL = BB_FREQ_CONTACTOR_CTRL;
	constexpr uint16_t BATT_CTRL = BB_FREQ_BATT_CTRL;
} // can_freq


/**
 *  Control Mode
 *  - Packet data for bb::can_id::CTRL_LINK
 */
namespace control_mode_id
{
	constexpr uint8_t AUTO = BB_CTRL_MODE_AUTO;
	constexpr uint8_t RC = BB_CTRL_MODE_RC;
	constexpr uint8_t STATION_KEEP = BB_CTRL_MODE_STATION_KEEP;
} // control_mode_id

/*
 *	MHPB (Main Hull Power Board) Channel Bit POS
 *	- Packet data for bb::can_id::MHPB_CTRL and bb::can_id::MHPB_STATUS
 */
namespace mhpb_power_bit_pos
{
	constexpr uint8_t SWITCH =      BB_MHPB_BITPOS_SWITCH;
	constexpr uint8_t LOGIC =       BB_MHPB_BITPOS_LOGIC;
	constexpr uint8_t PUMP =        BB_MHPB_BITPOS_PUMP;
	constexpr uint8_t EXTRA_5V =    BB_MHPB_BITPOS_EXTRA_5V;
    constexpr uint8_t OUSTER =      BB_MHPB_BITPOS_OUSTER;
	constexpr uint8_t EXTRA_24V =   BB_MHPB_BITPOS_EXTRA_24V;
	constexpr uint8_t NAV_HULL =    BB_MHPB_BITPOS_NAV_HULL;
	constexpr uint8_t PSU =         BB_MHPB_BITPOS_PSU;
} // mhpb_power_bit_pos

/*
 *	MHPB (Main Hull Power Board) Power Control from PC
 *	- Packet data for bb::can_id::MHPB_CTRL
 */
namespace mhpb_power_ctrl_pc
{
    // Enable Request for Power Channels
    // Request to turn off Switch, Logic, Pump(& fans) and PSU will be ignored
    // 0 = request to enable, 1 = request to disable
	constexpr uint8_t EN_SWITCH =      BB_MHPB_POWER_CTRL_EN_BIT_SWITCH;
	constexpr uint8_t EN_LOGIC =       BB_MHPB_POWER_CTRL_EN_BIT_LOGIC;
	constexpr uint8_t EN_PUMP =        BB_MHPB_POWER_CTRL_EN_BIT_PUMP;
	constexpr uint8_t EN_EXTRA_5V =    BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_5V;
    constexpr uint8_t EN_OUSTER =      BB_MHPB_POWER_CTRL_EN_BIT_OUSTER;
	constexpr uint8_t EN_EXTRA_24V =   BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_24V;
	constexpr uint8_t EN_NAV_HULL =    BB_MHPB_POWER_CTRL_EN_BIT_NAV_HULL;
	constexpr uint8_t EN_PSU =         BB_MHPB_POWER_CTRL_EN_BIT_PSU;

    // Power Cycle Request for Power Channels
    // Request to cycle PSU will be ignored
    // 0 = idle, 1 = request to cycle
    constexpr uint8_t CYCLE_SWITCH =      BB_MHPB_POWER_CTRL_CYCLE_BIT_SWITCH;
	constexpr uint8_t CYCLE_LOGIC =       BB_MHPB_POWER_CTRL_CYCLE_BIT_LOGIC;
	constexpr uint8_t CYCLE_PUMP =        BB_MHPB_POWER_CTRL_CYCLE_BIT_PUMP;
	constexpr uint8_t CYCLE_EXTRA_5V =    BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_5V;
    constexpr uint8_t CYCLE_OUSTER =      BB_MHPB_POWER_CTRL_CYCLE_BIT_OUSTER;
	constexpr uint8_t CYCLE_EXTRA_24V =   BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_24V;
	constexpr uint8_t CYCLE_NAV_HULL =    BB_MHPB_POWER_CTRL_CYCLE_BIT_NAV_HULL;
	constexpr uint8_t CYCLE_PSU =         BB_MHPB_POWER_CTRL_CYCLE_BIT_PSU;

    // Shutdown Request for PC and NAV
    // Request to cycle NAV will be ignored if NAV shutdown flag is not sent along in the same packet
    // PC shutdown flag is to pressing the button
    // 0 = idle, 1 = request to shutdown
    constexpr uint8_t SHUTDOWN_NAV_HULL =    BB_MHPB_SHUTDOWN_REQUEST_BIT_NAV_HULL;
	constexpr uint8_t SHUTDOWN_PC =          BB_MHPB_SHUTDOWN_REQUEST_BIT_PC;
} // mhpb_power_ctrl_pc

/*
 *	MHPB (Main Hull Power Board) Power Control from RC
 *	- Packet data for bb::can_id::MHPB_CTRL
 */
namespace mhpb_power_ctrl_rc
{
    // Enable Request for Power Channels
    // Request to turn off Switch, Logic, Pump(& fans) and PSU will be ignored
    // 0 = request to enable, 1 = request to disable
	// constexpr uint8_t EN_SWITCH =      BB_MHPB_POWER_CTRL_EN_BIT_SWITCH;
	// constexpr uint8_t EN_LOGIC =       BB_MHPB_POWER_CTRL_EN_BIT_LOGIC;
	// constexpr uint8_t EN_PUMP =        BB_MHPB_POWER_CTRL_EN_BIT_PUMP;
	// constexpr uint8_t EN_EXTRA_5V =    BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_5V;
    // constexpr uint8_t EN_OUSTER =      BB_MHPB_POWER_CTRL_EN_BIT_OUSTER;
	// constexpr uint8_t EN_EXTRA_24V =   BB_MHPB_POWER_CTRL_EN_BIT_EXTRA_24V;
	// constexpr uint8_t EN_NAV_HULL =    BB_MHPB_POWER_CTRL_EN_BIT_NAV_HULL;
	// constexpr uint8_t EN_PSU =         BB_MHPB_POWER_CTRL_EN_BIT_PSU;

    // Power Cycle Request for Power Channels
    // Request to cycle PSU will be ignored
    // 0 = idle, 1 = request to cycle
    constexpr uint8_t CYCLE_SWITCH =      BB_MHPB_POWER_CTRL_CYCLE_BIT_SWITCH;
	constexpr uint8_t CYCLE_LOGIC =       BB_MHPB_POWER_CTRL_CYCLE_BIT_LOGIC;
	constexpr uint8_t CYCLE_PUMP =        BB_MHPB_POWER_CTRL_CYCLE_BIT_PUMP;
	constexpr uint8_t CYCLE_EXTRA_5V =    BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_5V;
    constexpr uint8_t CYCLE_OUSTER =      BB_MHPB_POWER_CTRL_CYCLE_BIT_OUSTER;
	constexpr uint8_t CYCLE_EXTRA_24V =   BB_MHPB_POWER_CTRL_CYCLE_BIT_EXTRA_24V;
	constexpr uint8_t CYCLE_NAV_HULL =    BB_MHPB_POWER_CTRL_CYCLE_BIT_NAV_HULL;
	constexpr uint8_t CYCLE_PSU =         BB_MHPB_POWER_CTRL_CYCLE_BIT_PSU;

    // Shutdown Request for PC and NAV
    // Request to cycle NAV will be ignored if NAV shutdown flag is not sent along in the same packet
    // PC shutdown flag is to pressing the button
    // 0 = idle, 1 = request to shutdown
    // constexpr uint8_t SHUTDOWN_NAV_HULL =    BB_MHPB_SHUTDOWN_REQUEST_BIT_NAV_HULL;
	// constexpr uint8_t SHUTDOWN_PC =          BB_MHPB_SHUTDOWN_REQUEST_BIT_PC;
} // mhpb_power_ctrl_rc

/*
 *	MHPB (Main Hull Power Board) Power Status
 *	- Packet data for bb::can_id::MHPB_STATUS
 */
namespace mhpb_power_stats
{
    // Enable status of the channels
    // 0 = channel is enabled, 1 = channel is disabled
	constexpr uint8_t EN_SWITCH =      BB_MHPB_POWER_STATS_EN_BIT_SWITCH;
	constexpr uint8_t EN_LOGIC =       BB_MHPB_POWER_STATS_EN_BIT_LOGIC;
	constexpr uint8_t EN_PUMP =        BB_MHPB_POWER_STATS_EN_BIT_PUMP;
	constexpr uint8_t EN_EXTRA_5V =    BB_MHPB_POWER_STATS_EN_BIT_EXTRA_5V;
    constexpr uint8_t EN_OUSTER =      BB_MHPB_POWER_STATS_EN_BIT_OUSTER;
	constexpr uint8_t EN_EXTRA_24V =   BB_MHPB_POWER_STATS_EN_BIT_EXTRA_24V;
	constexpr uint8_t EN_NAV_HULL =    BB_MHPB_POWER_STATS_EN_BIT_NAV_HULL;
	constexpr uint8_t EN_PSU =         BB_MHPB_POWER_STATS_EN_BIT_PSU;

    // Cycle status of the channels (power cycle can be induced from request or attempt to fault recover)
    // 0 = idle, 1 = power cycling
    constexpr uint8_t CYCLE_SWITCH =      BB_MHPB_POWER_STATS_CYCLE_BIT_SWITCH;
	constexpr uint8_t CYCLE_LOGIC =       BB_MHPB_POWER_STATS_CYCLE_BIT_LOGIC;
	constexpr uint8_t CYCLE_PUMP =        BB_MHPB_POWER_STATS_CYCLE_BIT_PUMP;
	constexpr uint8_t CYCLE_EXTRA_5V =    BB_MHPB_POWER_STATS_CYCLE_BIT_EXTRA_5V;
    constexpr uint8_t CYCLE_OUSTER =      BB_MHPB_POWER_STATS_CYCLE_BIT_OUSTER;
	constexpr uint8_t CYCLE_EXTRA_24V =   BB_MHPB_POWER_STATS_CYCLE_BIT_EXTRA_24V;
	constexpr uint8_t CYCLE_NAV_HULL =    BB_MHPB_POWER_STATS_CYCLE_BIT_NAV_HULL;
	constexpr uint8_t CYCLE_PSU =         BB_MHPB_POWER_STATS_CYCLE_BIT_PSU;

    // Fault status of the channels
    // 0 = good, 1 = bad
    constexpr uint8_t FAULT_SWITCH =      BB_MHPB_POWER_STATS_FAULT_BIT_SWITCH;
	constexpr uint8_t FAULT_LOGIC =       BB_MHPB_POWER_STATS_FAULT_BIT_LOGIC;
	constexpr uint8_t FAULT_PUMP =        BB_MHPB_POWER_STATS_FAULT_BIT_PUMP;
	constexpr uint8_t FAULT_EXTRA_5V =    BB_MHPB_POWER_STATS_FAULT_BIT_EXTRA_5V;
    constexpr uint8_t FAULT_OUSTER =      BB_MHPB_POWER_STATS_FAULT_BIT_OUSTER;
	constexpr uint8_t FAULT_EXTRA_24V =   BB_MHPB_POWER_STATS_FAULT_BIT_EXTRA_24V;
	constexpr uint8_t FAULT_NAV_HULL =    BB_MHPB_POWER_STATS_FAULT_BIT_NAV_HULL;
	constexpr uint8_t FAULT_PSU =         BB_MHPB_POWER_STATS_FAULT_BIT_PSU;
} // mhpb_power_stats

/*
 *  Logic Board Power Status/Control Bits
 *  - Packet data for bb:can_id::LOGIC_CTRL & bb:can_id::LOGIC_STATUS
 */
namespace logic_power_bit
{
	constexpr uint8_t TELEM_BOARD = BB_LOGIC_POWER_BIT_TELEM_BOARD;
	constexpr uint8_t THRUSTERS_BOARD = BB_LOGIC_POWER_BIT_THRUSTERS_BOARD;
} // logic_power_bit

/*
 *  Kill Bits
 *  - Packet data for bb::can_id::KILL_STATUS
 */
namespace kill_bit
{
	constexpr uint8_t HARD = BB_KILL_BIT_HARD;
	constexpr uint8_t PC = BB_KILL_BIT_PC;
	constexpr uint8_t RC = BB_KILL_BIT_RC;
	constexpr uint8_t TELEM_NO_HB = BB_KILL_BIT_TELEM_NO_HB;
	constexpr uint8_t ESC1 = BB_KILL_BIT_ESC1;
	constexpr uint8_t ESC2 = BB_KILL_BIT_ESC2;
} // kill_bit

/*
 * Thruster Control Bits
 * - Packet data for bb::can_id::RC_THRUSTER & bb::can_id::PC_THRUSTER
 */
namespace thruster_ctrl
{
	constexpr uint16_t REVERSE_MAX = BB_THRUSTER_REVERSE_MAX;
	constexpr uint16_t STOP = BB_THRUSTER_STOP;
	constexpr uint16_t FORWARD_MAX = BB_THRUSTER_FORWARD_MAX;
} // rc_thruster & PC_thruster

/*
 *  Act Thrusters Azimuth Stats
 *  - Packet data for bb::can_id::ACT_AZI_STATS
 */
namespace act_azi_stats
{
	constexpr uint16_t ERROR = BB_ACT_AZI_STATS_ERROR;
	constexpr uint16_t CALIBRATING = BB_ACT_AZI_STATS_CALIBRATING;
	constexpr uint16_t LEFT = BB_ACT_AZI_STATS_LEFT;
	constexpr uint16_t RIGHT = BB_ACT_AZI_STATS_RIGHT;
} // act_azi_stats_bit

/*
 *  Act Thrusters Vertical Stats
 *  - Packet data for bb::can_id::ACT_VERT_STATS
  */
namespace act_vert_stats
{
	constexpr uint8_t ERROR = BB_ACT_VERT_STATS_ERROR;
	constexpr uint8_t CALIBRATING = BB_ACT_VERT_STATS_CALIBRATING;
	constexpr uint8_t LOWEST = BB_ACT_VERT_STATS_LOWEST;
	constexpr uint8_t HIGHEST = BB_ACT_VERT_STATS_HIGHEST;
} // act_vert_stats_bit

/*
 *	ACT BS Stats bits
 *	- Packet data for bb::can_id::ACT_BS_STATS
 */
namespace act_bs_stats
{
	constexpr uint8_t LOADER_ERROR = BB_BS_STATS_BALL_LOADER_ERROR;
	constexpr uint8_t LOADER_UNLOADED = BB_BS_STATS_BALL_LOADER_UNLOADED;
	constexpr uint8_t LOADER_LOADED = BB_BS_STATS_BALL_LOADER_LOADED;
	constexpr uint16_t TILT_ERROR = BB_BS_STATS_TILT_ERROR;
	constexpr uint16_t TILT_DOWN = BB_BS_STATS_TILT_DOWN;
	constexpr uint16_t TILT_UP = BB_BS_STATS_TILT_UP;
	constexpr uint16_t PAN_ERROR = BB_BS_STATS_PAN_ERROR;
	constexpr uint16_t PAN_LEFT = BB_BS_STATS_PAN_LEFT;
	constexpr uint16_t PAN_RIGHT = BB_BS_STATS_PAN_RIGHT;
	constexpr uint16_t FLYWHEEL_ERROR = BB_BS_STATS_FLYWHEEL_ERROR;
	constexpr uint16_t FLYWHEEL_OFF = BB_BS_STATS_FLYWHEEL_OFF;
	constexpr uint16_t FLYWHEEL_FORWARD_MAX = BB_BS_STATS_FLYWHEEL_FORWARD_MAX;
} // act_bs_stats_bit

/*
 * RC Vertical Actuator Control Bits
 * - Packet data for bb::can_id::RC_ACTUATION_VERT
*/
namespace rc_act_vert_ctrl
{
	constexpr uint8_t IGNORE = BB_RC_ACTUATION_VERT_IGNORE;
	constexpr uint8_t CALIBRATE = BB_RC_ACTUATION_VERT_CALIBRATE;
	constexpr uint8_t LOWEST = BB_RC_ACTUATION_VERT_LOWEST;
	constexpr uint8_t HIGHEST = BB_RC_ACTUATION_VERT_HIGHEST;
} // rc_act_vert_ctrl

/*
 * RC Thruster Azimuth Control Bits
 * - Packet data for bb::can_id::RC_THRUSTER_AZI
 */
namespace rc_thruster_azi_ctrl
{
	constexpr uint16_t IGNORE = BB_RC_THRUSTER_AZI_IGNORE;
	constexpr uint16_t CALIBRATE = BB_RC_THRUSTER_AZI_CALIBRATE;
	constexpr uint16_t LEFT = BB_RC_THRUSTER_AZI_LEFT;
	constexpr uint16_t RIGHT = BB_RC_THRUSTER_AZI_RIGHT;
} // rc_thruster_azi_ctrl

/*
 * PC Vertical Actuator Control Bits
 * - Packet data for bb::can_id::PC_ACTUATION_VERT
 */
namespace pc_act_vert_ctrl
{
	constexpr uint8_t IGNORE = BB_PC_ACTUATION_VERT_IGNORE;
	constexpr uint8_t CALIBRATE = BB_PC_ACTUATION_VERT_CALIBRATE;
	constexpr uint8_t LOWEST = BB_PC_ACTUATION_VERT_LOWEST;
	constexpr uint8_t HIGHEST = BB_PC_ACTUATION_VERT_HIGHEST;
} // pc_act_vert_ctrl

/*
 * PC Thruster Azimuth Control Bits
 * - Packet data for bb::can_id::PC_THRUSTER_AZI
 */
namespace pc_thruster_azi_ctrl
{
	constexpr uint16_t IGNORE = BB_PC_THRUSTER_AZI_IGNORE;
	constexpr uint16_t CALIBRATE = BB_PC_THRUSTER_AZI_CALIBRATE;
	constexpr uint16_t LEFT = BB_PC_THRUSTER_AZI_LEFT;
	constexpr uint16_t RIGHT = BB_PC_THRUSTER_AZI_RIGHT;
} // pc_thruster_azi_ctrl

/*
 *	RC Ballshooter control
 *	- Packet data for bb::can_id::RC_BALLSHOOTER
 */
namespace rc_ballshooter_ctrl
{
	constexpr uint8_t IGNORE = BB_RC_BALLSHOOTER_LOADER_IGNORE;
	constexpr uint8_t LOAD = BB_RC_BALLSHOOTER_LOADER_LOAD;
	constexpr uint8_t SHOOT = BB_RC_BALLSHOOTER_LOADER_SHOOT;
	constexpr uint16_t TILT_IGNORE = BB_RC_BALLSHOOTER_TILT_IGNORE;
	constexpr uint16_t TILT_CALIBRATE = BB_RC_BALLSHOOTER_TILT_CALIBRATE;
	constexpr uint16_t TILT_DOWN = BB_RC_BALLSHOOTER_TILT_DOWN;
	constexpr uint16_t TILT_UP = BB_RC_BALLSHOOTER_TILT_UP;
	constexpr uint16_t PAN_IGNORE = BB_RC_BALLSHOOTER_PAN_IGNORE;
	constexpr uint16_t PAN_CALIBRATE = BB_RC_BALLSHOOTER_PAN_CALIBRATE;
	constexpr uint16_t PAN_LEFT = BB_RC_BALLSHOOTER_PAN_LEFT;
	constexpr uint16_t PAN_RIGHT = BB_RC_BALLSHOOTER_PAN_RIGHT;
	constexpr uint16_t FLYWHEEL_IGNORE = BB_RC_BALLSHOOTER_FLYWHEEL_IGNORE;
	constexpr uint16_t FLYWHEEL_OFF = BB_RC_BALLSHOOTER_FLYWHEEL_OFF;
	constexpr uint16_t FLYWHEEL_FORWARD_MAX = BB_RC_BALLSHOOTER_FLYWHEEL_FORWARD_MAX;
} // rc_ballshooter_ctrl

/*
 *	ACT BS ball loader control bits (PC)
 *	- Packet data for bb::can_id::PC_BALLSHOOTER
 */
namespace pc_ballshooter_ctrl
{
	constexpr uint8_t IGNORE = BB_PC_BALLSHOOTER_LOADER_IGNORE;
	constexpr uint8_t LOAD = BB_PC_BALLSHOOTER_LOADER_LOAD;
	constexpr uint8_t SHOOT = BB_PC_BALLSHOOTER_LOADER_SHOOT;
	constexpr uint16_t TILT_IGNORE = BB_PC_BALLSHOOTER_TILT_IGNORE;
	constexpr uint16_t TILT_CALIBRATE = BB_PC_BALLSHOOTER_TILT_CALIBRATE;
	constexpr uint16_t TILT_DOWN = BB_PC_BALLSHOOTER_TILT_DOWN;
	constexpr uint16_t TILT_UP = BB_PC_BALLSHOOTER_TILT_UP;
	constexpr uint16_t PAN_IGNORE = BB_PC_BALLSHOOTER_PAN_IGNORE;
	constexpr uint16_t PAN_CALIBRATE = BB_PC_BALLSHOOTER_PAN_CALIBRATE;
	constexpr uint16_t PAN_LEFT = BB_PC_BALLSHOOTER_PAN_LEFT;
	constexpr uint16_t PAN_RIGHT = BB_PC_BALLSHOOTER_PAN_RIGHT;
	constexpr uint16_t FLYWHEEL_IGNORE = BB_PC_BALLSHOOTER_FLYWHEEL_IGNORE;
	constexpr uint16_t FLYWHEEL_OFF = BB_PC_BALLSHOOTER_FLYWHEEL_OFF;
	constexpr uint16_t FLYWHEEL_FORWARD_MAX = BB_PC_BALLSHOOTER_FLYWHEEL_FORWARD_MAX;
} // pc_ballshooter_ctrl

} // bb
