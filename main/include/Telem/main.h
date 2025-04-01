#pragma once
#include <stdint.h>
#include "Telem/define.h"

// RC Variables
// enums for switches and shits
/* CONTORL INPUTS */
enum class RC_Bistate
{
	AWAY,
	TOWARDS
};

enum class RC_Tristate
{
	AWAY,
	MID,
	TOWARDS
};

enum class RC_Gimbal_Mode
{
	THRUSTERS_MODE,
	BALLSHOOTER_MODE
};

enum class RC_Act_Lock
{
	LOCKED,
	UNLOCKED
};

enum class RC_Kill : uint8_t
{
	OPERATIONAL = 0,
	KILLED = 1
	// should be in can.hpp instead
};

enum class RC_Vehicle_Mode : uint8_t
{
	RC_CTRL_MODE = control_mode_id::RC,
	STATION_KEEP = control_mode_id::STATION_KEEP,
	AUTONOMOUS = control_mode_id::AUTO
};

enum class RC_Thrust_Deployment : uint8_t
{
	RETRACTED = rc_act_vert_ctrl::HIGHEST,
	DEPLOYED = rc_act_vert_ctrl::LOWEST
};

enum class RC_Hydro_Deployment : uint8_t
{
	RETRACTED = rc_act_vert_ctrl::HIGHEST,
	DEPLOYED = rc_act_vert_ctrl::LOWEST
};

enum class RC_Load_Shoot : uint8_t
{
	IDLE = rc_ballshooter_ctrl::IGNORE,
	LOAD = rc_ballshooter_ctrl::LOAD,
	SHOOT = rc_ballshooter_ctrl::SHOOT
};

enum class RC_Act_Cal : uint8_t
{   
    IDLE,
	PB_AZI,
    PQ_AZI,
    SB_AZI,
    SQ_AZI,
    PAN,
    TILT,
    PB_VERT,
    PQ_VERT,
    SB_VERT,
    SQ_VERT,
    HY_VERT
};

enum class RC_Thruster_Azi
{
	STATION_KEEP,
	UNDER_WAY
};

/* TELEMETRY OUTPUTS */
enum class RC_Telem_Kill : uint8_t
{
    NONE = 0b00,
    HARD_KILLED = 0b01,
    PC_KILLED = 0b10,

    // both RC and NO_TELEM_HB_KILLED are considered to be the same from the Tx perspective
    // No more bits to spare
    RC_KILLED = 0b11,
    NO_TELEM_HB_KILLED = 0b11
};

enum class RC_Telem_Vert_Act : uint8_t
{
    RETRACTED = 0,
    MOVING = 1,
    DEPLOYED = 2,
    CALIBRATING,
    ERROR
};

CRSF crsf;
int16_t ocs_rssi = 0;
int16_t rc_rssi = 0;

RC_Gimbal_Mode rcGimbalMode = RC_Gimbal_Mode::THRUSTERS_MODE;
RC_Act_Lock rcActLock = RC_Act_Lock::LOCKED;

RC_Kill rcKill = RC_Kill::KILLED;
RC_Vehicle_Mode rcVehMode = RC_Vehicle_Mode::RC_CTRL_MODE;
RC_Thrust_Deployment rcThrustDeploy = RC_Thrust_Deployment::RETRACTED;
RC_Hydro_Deployment rcHydroDeploy = RC_Hydro_Deployment::RETRACTED;
RC_Load_Shoot rcLoadShoot = RC_Load_Shoot::IDLE;
RC_Act_Cal rcActCal = RC_Act_Cal::IDLE;
RC_Thruster_Azi rcThrustAzi = RC_Thruster_Azi::STATION_KEEP;
uint8_t rcMHPBCycle = 0x00;

uint16_t rcCanPortBow = thruster_ctrl::STOP;
uint16_t rcCanStarBow = thruster_ctrl::STOP;
uint16_t rcCanPortQtr = thruster_ctrl::STOP;
uint16_t rcCanStarQtr = thruster_ctrl::STOP;

uint16_t rcCanPan = rc_ballshooter_ctrl::PAN_IGNORE;
uint16_t rcCanTilt = rc_ballshooter_ctrl::TILT_IGNORE;
uint16_t rcCanFlywheel = rc_ballshooter_ctrl::FLYWHEEL_OFF;

unsigned long rcTelemLastSend = 0;

// CAN variables
MCP_CAN CAN(CAN_CS);
uint32_t CAN_msg_id = 0;
uint8_t CAN_msg_len = 0; // length of CAN message, taken care by library
uint8_t CAN_msg_buf[8];  // Buffer for CAN message

struct heartbeat_t
{
	bool hb = false;
	unsigned long lastReceived = 0;
};

struct heartbeat_stats_t
{
	heartbeat_t pcHB;
	heartbeat_t lbkpHB;
	heartbeat_t mhpbHB;
	heartbeat_t thHB;
	heartbeat_t telemHB;
	heartbeat_t rcHB;
	heartbeat_t popbHB;
	heartbeat_t esc1HB;
	heartbeat_t esc2HB;
	heartbeat_t batt1HB;
	heartbeat_t batt2HB;
	heartbeat_t actHB;
	heartbeat_t navHB;
};

heartbeat_stats_t hbStats;

struct battery_stats_t
{
	uint16_t battVolt = 0;
	uint16_t battCurr = 0;
	uint16_t battTemp = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

battery_stats_t batt1Stats;
battery_stats_t batt2Stats;

struct popb_stats_t
{
	uint16_t popbChVolt = 0;
	uint16_t popbChCurr = 0;
	uint16_t popbChPower = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

popb_stats_t popbPcStats;
popb_stats_t popbRestStats;

struct mhpb_stats_t
{
	uint8_t mhpbEn = 0;
	uint8_t mhpbCycle = 0;
	uint8_t mhpbFault = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

mhpb_stats_t mhpbStats;

struct hull_climate_stats_t
{
	uint16_t temp = 0;
	uint16_t hum = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

hull_climate_stats_t mainHullClimateStats;
hull_climate_stats_t powerHullClimateStats;
hull_climate_stats_t actHullClimateStats;
hull_climate_stats_t navHullClimateStats;

struct pc_temp_t
{
	uint16_t cpuTemp = 0;
	uint16_t gpuTemp = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

pc_temp_t pcTempStats;

struct thruster_azi_stats_t
{
	uint16_t portBow = 0;
	uint16_t portQtr = 0;
	uint16_t starBow = 0;
	uint16_t starQtr = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

thruster_azi_stats_t thrusterAziStats;

struct vert_act_stats_t
{
	uint8_t portBow = 0;
	uint8_t portQtr = 0;
	uint8_t starBow = 0;
	uint8_t starQtr = 0;
	uint8_t hydrophone = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

vert_act_stats_t vertActStats;

struct ballshooter_stats_t
{
	uint16_t pan = 0;
	uint16_t tilt = 0;
	uint8_t loader = act_bs_stats::LOADER_ERROR;
	uint16_t flywheel = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

ballshooter_stats_t ballshooterStats;

struct kill_status_t
{
	RC_Kill rcKill = RC_Kill::KILLED;
	RC_Kill pcKill = RC_Kill::KILLED;
	RC_Kill hardKill = RC_Kill::KILLED;
	RC_Kill noTelemHBKill = RC_Kill::KILLED;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

kill_status_t killStats;

struct esc_stats_t
{
	uint16_t bowCurr = 0;
	uint16_t qtrCurr = 0;

	unsigned long lastReceived = 0;
    bool isValid = false;
};

esc_stats_t portEscStats;
esc_stats_t starEscStats;

// Screen variables
bool screen_exist = false;
LCD screen = LCD(SCREEN_CS, SCREEN_RESET);

uint32_t screen_prev_switch = 0;
uint32_t screen_prev_update = 0;
uint32_t self_heartbeat_prev_update = 0;
uint32_t can_kill_prev_send = 0;
uint32_t can_ctrl_link_prev_send = 0;
uint32_t can_mhpb_prev_send = 0;
uint32_t can_thrusters_prev_send = 0;
uint32_t can_calibration_prev_send = 0;
uint32_t can_thrust_hydro_deploy_prev_send = 0;
uint32_t can_thrust_azi_prev_send = 0;
uint32_t can_balls_shooter_prev_send = 0;

// RC functions
void rssiLeds(int8_t rssi, bool isTxConnected);
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
                          RC_Vehicle_Mode rcVehMode);

RC_Gimbal_Mode rcGetGimbalMode();

RC_Kill rcGetKill();
RC_Vehicle_Mode rcGetVehicleControlMode();
RC_Thrust_Deployment rcGetThrustDeploy();
RC_Hydro_Deployment rcGetHydroDeploy();
RC_Act_Lock rcGetActLock();
RC_Load_Shoot rcGetLoadShoot();
RC_Act_Cal rcGetActCal();
RC_Thruster_Azi rcGetThrustAziMode();
uint8_t rcGetMHPBCycle();

void rcGetBallshooterVal(uint16_t& canPan, uint16_t& canTilt, uint16_t& canFlywheel);
void rcGetThrusterVal(uint16_t& canPortBow, uint16_t& canStarBow, uint16_t& canPortQtr, uint16_t& canStarQtr);
uint16_t rcGetStickMinDeadZone(uint8_t channel);
uint16_t rcGetStickMidDeadZone(uint8_t channel);
RC_Bistate rcGetBistateVal(uint8_t channel);
RC_Tristate rcGetTristateVal(uint8_t channel);

// CAN functions
void initCAN(void);
void setCANMask(void);
void checkCANMsg(void);
void publishCAN(void);
void publishCANHeartbeat(uint8_t);
void publishKill(RC_Kill);
void publishCtrlLink(RC_Vehicle_Mode, int8_t);
void publishThrusters(uint16_t, uint16_t, uint16_t, uint16_t);
void publishThrustHydroDeploy(RC_Thrust_Deployment, RC_Hydro_Deployment, RC_Act_Cal);
void publishThrustAziMode(RC_Thruster_Azi, RC_Act_Cal);
void publishBallShooter(uint16_t, uint16_t, uint16_t, RC_Load_Shoot, RC_Act_Cal);
void publishMHPBCycle(uint8_t);

// Screen functions
void prepareScreen(void);
void updateScreenData(void);
void updateHeartbeat(void);
void resetStats(void);
void resetHeartBeat(void);
void resetMainHullStats(void);
void resetPowerHullStats(void);
void resetBatteryStats(void);
void resetPOPBStats(void);
void resetMHPBStats(void);
void resetSBCStats(void);
void resetActHullStats(void);
void resetNavHullStats(void);
void resetBallShooterStats(void);
void resetActAziStats(void);
void resetActVertStats(void);
