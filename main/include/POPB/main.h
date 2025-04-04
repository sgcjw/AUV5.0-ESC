#pragma once

// CAN Initialisation Functions
void CAN_init(void);
void set_CANMask(void);
void check_CANmsg(void);

// CAN Transmission Functions
void publish_CAN(void);
void publishCAN_heartbeat(uint8_t);
void publishCAN_Hardkill(void);
void publishPOPB_heartbeats(void);

// User-defined battery fuctions
void reset_batteryheartbeat(void);

//PMIC initialisation functions
void INA238_init(void); 

//User-defined button functions
void check_button(void);

//Battery control function
void off_onebattery(void);

//SHT4x functions
void SHT_init(void);
void SHT_read(void);

//Test ESC
void test_ESC(void);

