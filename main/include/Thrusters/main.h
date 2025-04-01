#pragma once
#include <stdint.h>

void can_init(void);
void setCANMask(void);
void checkCANmsg(void);
void publishCAN(void);
void publishCAN_heartbeat(int device_id);
void publish_heartbeats(void);
void publishKill_Status(void);
void requestESC_Stats(int esc_id);
void publishESC_Stats(int esc_id);
void requestESC_Fault_Flags(int esc_id);
void requestMotor_Status_Flags(int esc_id);
void publishESC_Flags(int esc_id);
void publishMH_Stats(void);
void publishThrusters(void);

void light_tower_init(void);
void control_light_tower(void);

void SHT_init(void);
void SHT_read(void);
void probe_read(void);
void sensors_read(void);

void control_kill(void);

void update_timeout(void);
void update_esc_heartbeat(void);
void update_kill(void);
void update_sensor_validity(void);