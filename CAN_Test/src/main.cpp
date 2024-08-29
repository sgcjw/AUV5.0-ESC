#include <Arduino.h>
#include <SPI.h>
#include <mcp2515.h>
#include <main.h>

/* CAN Read Message */
struct can_frame canTX;
struct can_frame canRX;
MCP2515 mcp2515(10);


void setup() {
  Serial.begin(115200);
  CAN_init();
}

void loop() {
  checkCANmsg();
  publishCAN();
  delay(1000);
}

void checkCANmsg(){
  if (mcp2515.readMessage(&canRX) == MCP2515::ERROR_OK) {
    Serial.print(canRX.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canRX.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canRX.can_dlc; i++)  {  // print the data
      Serial.print(canRX.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();
  } 
} 

void publishCAN(){
  mcp2515.sendMessage(&canTX);
  Serial.println("Messages sent");
}

void CAN_init(){
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  canTX.can_id  = 0x00;
  canTX.can_dlc = 8;
  canTX.data[0] = 0x00;
  canTX.data[1] = 0x00;
  canTX.data[2] = 0x00;
  canTX.data[3] = 0x00;
  canTX.data[4] = 0x00;
  canTX.data[5] = 0x00;
  canTX.data[6] = 0x00;
  canTX.data[7] = 0x00;  
}