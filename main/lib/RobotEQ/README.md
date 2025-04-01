About
=====

The Arduino-RobotEQ library is an abstraction interface for the RobotEQ Motor
Controllers. It uses the serial command inteface to send and receive commands
from the RobotEQ controller using an Arduino.

Note
====

For this repository, CAN ID for ROBOTEQ_CAN_SEND and ROBOTEQ_CAN_REPLY have
been defined in the define.h of the Thrusters code, with the inclusion being
called in "RobotEQ.cpp". When copying this library, ensure that the include path
is updated for your use, or remove the inclusion and uncomment the define in
"RobotEQ.h" and update the respective Node IDs.

Usage
=====

    #include <RobotEQ.h>

    #define CHANNEL_1 1

    // Configure Motor Controllers
    RobotEQ controller(&Serial);

    void setup() {
    }

    void loop() {
        int voltage;
        int amps;

        if (controller.isConnected()) {
            voltage = controller.queryBatteryVoltage();
            amps = controller.queryBatteryAmps();

            controller.commandMotorPower(CHANNEL_1, 1000);
        }
    }
