// ###################################################
//
// ____________________    _____    _____________   ____
// \______   \______   \  /  _  \  /   _____/\   \ /   /
//  |    |  _/|    |  _/ /  /_\  \ \_____  \  \   Y   /
//  |    |   \|    |   \/    |    \/        \  \     /
//  |______  /|______  /\____|__  /_______  /   \___/
//         \/        \/         \/        \/
//
// Main Hull Power Board for ASV 4.0
//
// <insert relevant info>
//
// Written By: Zi Yang
//
// Summary:
// Power control through enable pins of high side load siwtch for 24 V, buck for 12 V and boost for 54 V
// Power monitoring through power good signal from the above chips
//
// ###################################################

#include <Arduino.h>
#include <SPI.h>
#include <can.h>

#include "MHPB/define.h"
#include "MHPB/main.h"

#include "common/bb_can.h"
using namespace bb;

void setup() 
{

}

void loop()
{

}