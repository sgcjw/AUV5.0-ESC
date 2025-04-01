/*
credit to https://github.com/stepinside/Arduino-CRSF/tree/main
heavily modified by BBAS
*/
#include "Arduino_CRSF.h"

/**
 * @brief Init the CRSF serial port
 * @param port serial port
 * @param baud serial baudrate
 * @retval None
 */
void CRSF::begin(HardwareSerial* port, unsigned long baud)
{
	serialPort = port;
	uint8_t loc_crsfData[CRSF_MAX_RX_PACKET_SIZE] = { 0x00 };
	int16_t loc_channels[16] = { 1023 };

	serialPort->begin(baud);

	// initial data
	memcpy(crsfRxData, loc_crsfData, CRSF_MAX_RX_PACKET_SIZE);
	memcpy(crsfChannels, loc_channels, CRSF_MAX_CHANNEL);
}

/**
 * @brief returns no of bytes in serial buffer for CRSF
 * @param None
 * @retval no of bytes in serial buffer for CRSF
 */
int CRSF::serialBytesAvailable() const
{
	return serialPort->available();
}

/**
 * @brief Reads serial buffer to get RC channels data and link status data
 * @param None
 * @retval None
 */
void CRSF::readPacket()
{
	uint8_t frameAddress = 0;
	uint8_t frameLength = 0;
	uint8_t frameType = 0;

	uint8_t bufferIndex = 0;

	unsigned long crsfRxTimeout = millis();

	while (serialPort->available() > 0) {
		if (millis() - crsfRxTimeout > CRSF_SERIAL_TIMEOUT) {
			// incase stuck in while loop for too long
			// although tested it wont be stuck
			break;
		}

		if (bufferIndex == 0) {
			// 1st byte is device address, check its for the FC
			frameAddress = serialPort->read();

			if (frameAddress == CRSF_ADDRESS_FLIGHT_CONTROLLER) {
				// shove address in index 0
				crsfRxData[bufferIndex++] = frameAddress;

				// 2nd byte is frame length (include length for frame type, payload and crc)
				frameLength = serialPort->read();
				// shove frame length in index 1
				crsfRxData[bufferIndex++] = frameLength;

				// 3rd byte is frame type
				frameType = serialPort->read();
				// shove frame length in index 2
				crsfRxData[bufferIndex++] = frameType;
			}
			else {
				bufferIndex = 0; // message not for FC, discard
			}
		}
		else if (bufferIndex >= 3 && bufferIndex < frameLength + 1) {
			// fill buffer with payload from index 3 to index 12 for link stats
			// or
			// fill buffer with payload from index 3 to index 24 for RC channel
			crsfRxData[bufferIndex++] = serialPort->read();
		}
		else if (bufferIndex == frameLength + 1) {
			// last byte is CRC
			uint8_t frameCRC = serialPort->read();
			// shove frame CRC in last index (bufferIndex = frameLength + 1)
			crsfRxData[bufferIndex++] = frameCRC;

			// calculate received packet crc
			uint8_t calculatedCRC = crsf_crc8(&crsfRxData[2], frameLength - 1);

			// If crc is correct update link stats or channel accordingly
			if (calculatedCRC == frameCRC) {
				switch (frameType) {
				case CRSF_FRAMETYPE_LINK_STATISTICS:
					// double check frame length
					if (frameLength == CRSF_LINKSTATS_FRAME_LENGTH) {
						updateLinkStats();
						crsfLinkStatsLastReceived = millis();
					}
					break;
				case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
					// double check frame length
					if (frameLength == CRSF_RCCHANNEL_FRAME_LENGTH) {
						updateChannels();
						crsfRCChannelLastReceived = millis();
					}
					break;
				}
			}
		}
		else {
			// set index back to 0 to read again, until serial buffer is empty
			bufferIndex = 0;
		}
	}
}

/**
 * @brief updates the 16 channel when a new channel frame arrives and passes CRC
 * @param None
 * @retval None
 */
void CRSF::updateChannels()
{
	crsfPayloadRcChannelsPacked_s packedChannels;
	memcpy(&packedChannels, crsfRxData + 3, CRSF_RCCHANNEL_PAYLOAD_LENGTH);
	crsfChannels[0] = packedChannels.chan0;
	crsfChannels[1] = packedChannels.chan1;
	crsfChannels[2] = packedChannels.chan2;
	crsfChannels[3] = packedChannels.chan3;
	crsfChannels[4] = packedChannels.chan4;
	crsfChannels[5] = packedChannels.chan5;
	crsfChannels[6] = packedChannels.chan6;
	crsfChannels[7] = packedChannels.chan7;
	crsfChannels[8] = packedChannels.chan8;
	crsfChannels[9] = packedChannels.chan9;
	crsfChannels[10] = packedChannels.chan10;
	crsfChannels[11] = packedChannels.chan11;
	crsfChannels[12] = packedChannels.chan12;
	crsfChannels[13] = packedChannels.chan13;
	crsfChannels[14] = packedChannels.chan14;
	crsfChannels[15] = packedChannels.chan15;
}

/**
 * @brief updates the link statistics when a new link stats frame arrives and passes CRC
 * @param None
 * @retval None
 */
void CRSF::updateLinkStats()
{
	memcpy(&crsfLinkStats, crsfRxData + 3, CRSF_LINKSTATS_PAYLOAD_LENGTH);
}

/**
 * @brief magic crc stuff from library
 * @param ptr pointer to start of array
 * @param len len of data
 * @retval calculated CRC
 */
uint8_t CRSF::crsf_crc8(const uint8_t* ptr, uint8_t len) const
{
	static const uint8_t crsf_crc8tab[256] = {
		0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D, 0x52, 0x87,
		0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F, 0xA4, 0x71, 0xDB, 0x0E,
		0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9, 0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD,
		0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B, 0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
		0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0, 0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33,
		0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2, 0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA,
		0xEE, 0x3B, 0x91, 0x44, 0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69,
		0xC3, 0x16, 0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
		0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0, 0x4B, 0x9E,
		0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36, 0x19, 0xCC, 0x66, 0xB3,
		0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64, 0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59,
		0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F, 0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
		0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D, 0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A,
		0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB, 0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07,
		0x53, 0x86, 0x2C, 0xF9
	};

	uint8_t crc = 0;
	for (uint8_t i = 0; i < len; i++) {
		crc = crsf_crc8tab[crc ^ *ptr++];
	}
	return crc;
}

/**
 * @brief returns value of channel requested
 * @param channel 1 to 16
 * @retval channel value
 */
uint16_t CRSF::getChannel(uint8_t channel) const
{
	return crsfChannels[channel - 1];
}

/**
 * @brief returns receiver RSSI for antenna 1
 * @param None
 * @retval RSSI -128 to 0 in dBm, more possitive better
 */
int8_t CRSF::getRxRSSI1() const
{
	return -1 * (crsfLinkStats.uplink_RSSI_1);
}

/**
 * @brief returns receiver RSSI for antenna 2
 * @param None
 * @retval RSSI -128 to 0 in dBm, more possitive better
 */
int8_t CRSF::getRxRSSI2() const
{
	return -1 * (crsfLinkStats.uplink_RSSI_2);
}

/**
 * @brief returns receiver RSSI based on which antenna is used, in BBAS we celebrate true diversity
 * @param None
 * @retval RSSI -128 to 0 in dBm, more possitive better
 */
int8_t CRSF::getRxRSSI() const
{
	return -1 * (crsfLinkStats.active_antenna ? crsfLinkStats.uplink_RSSI_2 : crsfLinkStats.uplink_RSSI_1);
}

/**
 * @brief returns controller RSSI
 * @param None
 * @retval RSSI -128 to 0 in dBm, more possitive better
 */
int8_t CRSF::getTxRSSI() const
{
	return -1 * crsfLinkStats.downlink_RSSI;
}

/**
 * @brief returns which antenna is used
 * @param None
 * @retval 1 for Ant1, 2 for Ant2
 */
uint8_t CRSF::getActAnt() const
{
	return (crsfLinkStats.active_antenna ? 2 : 1);
}

/**
 * @brief returns which RF mode is used
 * @param None
 * @retval RF modes
 */
uint8_t CRSF::getRFMode() const
{
	return crsfLinkStats.rf_Mode;
}

/**
 * @brief returns receiver link quality
 * @param None
 * @retval link quality 0 - 100 in %, higher better
 */
uint8_t CRSF::getRxLinkQuality() const
{
	return crsfLinkStats.uplink_Link_quality;
}

/**
 * @brief returns controller link quality
 * @param None
 * @retval link quality 0 - 100 in %, higher better
 */
uint8_t CRSF::getTxLinkQuality() const
{
	return crsfLinkStats.downlink_Link_quality;
}

/**
 * @brief returns receiver SNR
 * @param None
 * @retval SNR in dB, higher better
 */
int8_t CRSF::getRxSNR() const
{
	return crsfLinkStats.uplink_SNR;
}

/**
 * @brief returns receiver SNR
 * @param None
 * @retval SNR in dB, higher better
 */
int8_t CRSF::getTxSNR() const
{
	return crsfLinkStats.downlink_SNR;
}

/**
 * @brief give connect status, true = connected
 * @param None
 * @retval bool 1 = connected
 */
bool CRSF::isConnected()
{
	// return the status of connection
	if (millis() - crsfRCChannelLastReceived > CRSF_CONNECTION_TIMEOUT) {
		txDisconnected = true;
	}
	else {
		txDisconnected = false;
	}

	return !txDisconnected;
}

/**
 * @brief the link stat is in received last t ms
 * @param None
 * @retval in ms
 */
unsigned long CRSF::getLinkStatsLastReceived() const
{
	return millis() - crsfLinkStatsLastReceived;
}

/**
 * @brief the RC channel is in received last t ms
 * @param None
 * @retval in ms
 */
unsigned long CRSF::getRCChannelLastReceived() const
{
	return millis() - crsfRCChannelLastReceived;
}

/**
 * @brief send telem back to controller: 0x02 GPS
 * Device address: (uint8_t) CRSF_ADDRESS_CRSF_RECEIVER or sync byte????
 * Frame length:   length in bytes including length Type (uint8_t) CRSF_GPS_FRAME_LENGTH
 * Type:           (uint8_t) CRSF_FRAMETYPE_GPS
 * Payload:
 * int32_t     Latitude ( degree / 10`000`000 )
 * int32_t     Longitude (degree / 10`000`000 )
 * uint16_t    Groundspeed ( km/h / 10 )
 * uint16_t    GPS heading ( degree / 100 )
 * uint16      Altitude ( meter ­1000m offset )
 * uint8_t     Satellites in use ( counter )
 * CRC
 * @param latitude Latitude ( degree / 10`000`000 )
 * @param longitude Longitude (degree / 10`000`000 )
 * @param groundSpeed Groundspeed ( km/h / 10 )
 * @param gpsHeading GPS heading ( degree / 100 )
 * @param altitude Altitude ( meter ­1000m offset )
 * @param satUsed Satellites in use ( counter )
 * @retval None
 */
void CRSF::writeGPSPacket(int32_t latitude,
                          int32_t longitude,
                          uint16_t groundSpeed,
                          uint16_t gpsHeading,
                          uint16_t altitude,
                          uint8_t satUsed)
{
	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = CRSF_GPS_FRAME_LENGTH;
	crsfTxData[2] = CRSF_FRAMETYPE_GPS;
	crsfTxData[3] = ((latitude >> 24) & 0xFFU);
	crsfTxData[4] = ((latitude >> 16) & 0xFFU);
	crsfTxData[5] = ((latitude >> 8) & 0xFFU);
	crsfTxData[6] = (latitude & 0xFFU);
	crsfTxData[7] = ((longitude >> 24) & 0xFFU);
	crsfTxData[8] = ((longitude >> 16) & 0xFFU);
	crsfTxData[9] = ((longitude >> 8) & 0xFFU);
	crsfTxData[10] = (longitude & 0xFFU);
	crsfTxData[11] = ((groundSpeed >> 8) & 0xFFU);
	crsfTxData[12] = (groundSpeed & 0xFFU);
	crsfTxData[13] = ((gpsHeading >> 8) & 0xFFU);
	crsfTxData[14] = (gpsHeading & 0xFFU);
	crsfTxData[15] = ((altitude >> 8) & 0xFFU);
	crsfTxData[16] = (altitude & 0xFFU);
	crsfTxData[17] = (satUsed & 0xFFU);
	crsfTxData[18] = crsf_crc8(&crsfTxData[2], CRSF_GPS_FRAME_LENGTH - 1);
	serialPort->write(crsfTxData, CRSF_GPS_FRAME_LENGTH + 2);
}

/**
 * @brief send telem back to controller: 0x08 Battery sensor
 * Device address: (uint8_t) CRSF_ADDRESS_CRSF_RECEIVER or sync byte????
 * Frame length:   length in bytes including length Type (uint8_t) CRSF_BATTSTATS_FRAME_LENGTH
 * Type:           (uint8_t) CRSF_FRAMETYPE_BATTERY_SENSOR
 * Payload:
 * uint16_t    Voltage ( mV * 100 )
 * uint16_t    Current ( mA * 100 )
 * uint24_t    Fuel ( drawn mAh )
 * uint8_t     Battery remaining ( percent )
 * CRC
 * @param batt_voltage Voltage ( mV * 100 )
 * @param batt_current Current ( mA * 100 )
 * @param fuel Fuel ( drawn mAh )
 * @param battPercent Battery remaining ( percent )
 * @retval None
 */
void CRSF::writeBatteryPacket(uint16_t batt_voltage, uint16_t batt_current, uint32_t fuel, uint8_t battPercent)
{
	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = CRSF_BATTSTATS_FRAME_LENGTH;
	crsfTxData[2] = CRSF_FRAMETYPE_BATTERY_SENSOR;
	crsfTxData[3] = ((batt_voltage >> 8) & 0xFFU);
	crsfTxData[4] = (batt_voltage & 0xFFU);
	crsfTxData[5] = ((batt_current >> 8) & 0xFFU);
	crsfTxData[6] = (batt_current & 0xFFU);
	crsfTxData[7] = ((fuel >> 16) & 0xFFU);
	crsfTxData[8] = ((fuel >> 8) & 0xFFU);
	crsfTxData[9] = (fuel & 0xFFU);
	crsfTxData[10] = (battPercent & 0xFFU);
	crsfTxData[11] = crsf_crc8(&crsfTxData[2], CRSF_BATTSTATS_FRAME_LENGTH - 1);
	serialPort->write(crsfTxData, CRSF_BATTSTATS_FRAME_LENGTH + 2);
}

/**
 * @brief send telem back to controller: 0x1E Attitude
 * Device address: (uint8_t) CRSF_ADDRESS_CRSF_RECEIVER
 * Frame length:   length in bytes including length Type (uint8_t) CRSF_ATTITUDE_FRAME_LENGTH
 * Type:           (uint8_t) CRSF_FRAMETYPE_ATTITUDE
 * Payload:
 * int16_t     Pitch angle ( rad / 10000 )
 * int16_t     Roll angle ( rad / 10000 )
 * int16_t     Yaw angle ( rad / 10000 )
 * CRC
 * @param pitch Pitch angle ( rad / 10000 )
 * @param roll Roll angle ( rad / 10000 )
 * @param yaw Yaw angle ( rad / 10000 )
 * @retval None
 */
void CRSF::writeAttitudePacket(uint16_t pitch, uint16_t roll, uint16_t yaw)
{
	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = CRSF_ATTITUDE_FRAME_LENGTH;
	crsfTxData[2] = CRSF_FRAMETYPE_ATTITUDE;
	crsfTxData[3] = ((pitch >> 8) & 0xFFU);
	crsfTxData[4] = (pitch & 0xFFU);
	crsfTxData[5] = ((roll >> 8) & 0xFFU);
	crsfTxData[6] = (roll & 0xFFU);
	crsfTxData[7] = ((yaw >> 8) & 0xFFU);
	crsfTxData[8] = (yaw & 0xFFU);
	crsfTxData[9] = crsf_crc8(&crsfTxData[2], CRSF_ATTITUDE_FRAME_LENGTH - 1);
	serialPort->write(crsfTxData, CRSF_ATTITUDE_FRAME_LENGTH + 2);
}

/**
 * @brief send telem back to controller: 0x21 Flight mode
 * Device address: (uint8_t) CRSF_ADDRESS_CRSF_RECEIVER
 * Frame length:   length in bytes including length Type (uint8_t) len + 3 (type, '\0' and crc)
 * Type:           (uint8_t) CRSF_FRAMETYPE_FLIGHT_MODE
 * Payload:
 * char (1 Bytes) * char length + '\0' ( Null terminated string )
 * CRC
 * @param p_charArray pointer to char array
 * @param msglen length of char array
 * @retval None
 */
void CRSF::writeFlightModePacket(char* p_charArray, uint8_t msglen)
{
	msglen = msglen > (CRSF_FLIGHT_MODE_MAX_FRAME_LENGTH - 3) ? (CRSF_FLIGHT_MODE_MAX_FRAME_LENGTH - 3) : msglen;

	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = msglen + 3;
	crsfTxData[2] = CRSF_FRAMETYPE_FLIGHT_MODE;

	for (int i = 0; i < msglen; i++) {
		crsfTxData[i + 3] = *(p_charArray + i);
	}

	crsfTxData[3 + msglen] = '\0'; // append ending character
	crsfTxData[3 + msglen + 1] = crsf_crc8(&crsfTxData[2], msglen + 2);
	serialPort->write(crsfTxData, msglen + 3 + 2);
}

/*
0x29 Device Info
Payload:
uint8_t     Destination
uint8_t     Origin
char[]      Device Name ( Null terminated string )
uint32_t    Null Bytes
uint32_t    Null Bytes
uint32_t    Null Bytes
uint8_t     255 (Max MSP Parameter)
uint8_t     0x01 (Parameter version 1)
*/
void CRSF::crsfFrameDeviceInfo(char* p_charArray, uint8_t msglen)
{
	// crsfTxData[0] = 0;
	// crsfTxData[1] = CRSF_FRAMETYPE_DEVICE_INFO;
	// crsfTxData[2] = CRSF_ADDRESS_RADIO;
	// crsfTxData[3] = CRSF_ADDRESS_FLIGHT_CONTROLLER;

	// for (int i = 0; i < msglen; i++) {
	// 	crsfTxData[i + 4] = *(p_charArray + i);
	// }

	// crsfTxData[4 + msglen] = '\0'; // append ending character

	// // null bytes
	// for (int i=0; i<12; i++) {
	//     crsfTxData[5 + msglen + i] = 0x00;
	// }

	// crsfTxData[17 + msglen] = 0x00;
	// crsfTxData[18 + msglen] = 0x01;
	// crsfTxData[19 + msglen] = crsf_crc8(&crsfTxData[1], 18 + msglen);
	// serialPort->write(crsfTxData, 20 + msglen);

	// crsfTxData[0] = CRSF_FRAMETYPE_DEVICE_INFO;
	// crsfTxData[1] = CRSF_ADDRESS_RADIO;
	// crsfTxData[2] = CRSF_ADDRESS_FLIGHT_CONTROLLER;

	// for (int i = 0; i < msglen; i++) {
	//     crsfTxData[i + 3] = *(p_charArray + i);
	// }

	// crsfTxData[3 + msglen] = '\0'; // append ending character

	// // null bytes
	// for (int i=0; i<12; i++) {
	//     crsfTxData[4 + msglen + i] = 0x00;
	// }

	// crsfTxData[16 + msglen] = 0x00;
	// crsfTxData[17 + msglen] = 0x01;
	// crsfTxData[18 + msglen] = crsf_crc8(&crsfTxData[1], 17 + msglen);
	// serialPort->write(crsfTxData, 19 + msglen);

	// crsfTxData[0] = CRSF_ADDRESS_CRSF_RECEIVER;
	// crsfTxData[1] = 0;
	// crsfTxData[2] = CRSF_FRAMETYPE_DEVICE_INFO;
	// crsfTxData[3] = CRSF_ADDRESS_RADIO;
	// crsfTxData[4] = CRSF_ADDRESS_FLIGHT_CONTROLLER;

	// for (int i = 0; i < msglen; i++) {
	//     crsfTxData[i + 5] = *(p_charArray + i);
	// }

	// crsfTxData[5 + msglen] = '\0'; // append ending character

	// // null bytes
	// for (int i=0; i<12; i++) {
	//     crsfTxData[6 + msglen + i] = 0x00;
	// }

	// crsfTxData[18 + msglen] = 0x00;
	// crsfTxData[19 + msglen] = 0x01;
	// crsfTxData[20 + msglen] = crsf_crc8(&crsfTxData[2], 18 + msglen);
	// serialPort->write(crsfTxData, 21 + msglen);

	// crsfTxData[0] = CRSF_ADDRESS_CRSF_RECEIVER;
	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = 0;
	crsfTxData[2] = CRSF_FRAMETYPE_DEVICE_INFO;
	crsfTxData[3] = CRSF_ADDRESS_RADIO;
	crsfTxData[4] = CRSF_ADDRESS_FLIGHT_CONTROLLER;

	// msg
	crsfTxData[5] = 65;
	crsfTxData[6] = 66;

	crsfTxData[7] = '\0'; // append ending character

	// null bytes
	crsfTxData[8] = '\0';
	crsfTxData[9] = '\0';
	crsfTxData[10] = '\0';
	crsfTxData[11] = '\0';

	crsfTxData[12] = '\0';
	crsfTxData[13] = '\0';
	crsfTxData[14] = '\0';
	crsfTxData[15] = '\0';

	crsfTxData[16] = '\0';
	crsfTxData[17] = '\0';
	crsfTxData[18] = '\0';
	crsfTxData[19] = '\0';

	crsfTxData[20] = 0x00;
	crsfTxData[21] = 0x01;
	crsfTxData[22] = crsf_crc8(&crsfTxData[2], 20);
	serialPort->write(crsfTxData, 23);
}

void CRSF::crsfSendMspResponse()
{
	// crsfTxData[0] = CRSF_ADDRESS_CRSF_RECEIVER;
	crsfTxData[0] = CRSF_SYNC_BYTE;
	crsfTxData[1] = (58 + 4);
	crsfTxData[2] = 0x7B;
	crsfTxData[3] = CRSF_ADDRESS_RADIO;
	crsfTxData[4] = CRSF_ADDRESS_FLIGHT_CONTROLLER;

	for (int i = 0; i < 58; i++) {
		crsfTxData[i + 5] = i;
	}

	crsfTxData[58 + 5] = crsf_crc8(&crsfTxData[2], 58 + 3);
	serialPort->write(crsfTxData, 58 + 5 + 1);
}

// void CRSF::sendGarbaage()
// {
// 	// crsfTxData[0] = CRSF_ADDRESS_RADIO; // 0xEA
// 	crsfTxData[0] = CRSF_ADDRESS_CRSF_RECEIVER;    // 0xEC
// 	crsfTxData[1] = CRSF_FLIGHT_MODE_FRAME_LENGTH; // 0x3E
// 	crsfTxData[2] = CRSF_FRAMETYPE_FLIGHT_MODE;    // 0x7B
// 	crsfTxData[3] = 'A';                           // 0xEA
// 	crsfTxData[4] = 'B';                           // 0xC8
// 	crsfTxData[5] = 'C';                           // 0x05
// 	crsfTxData[6] = 'D';                           // 0x05
// 	crsfTxData[7] = 'E';                           // 0x05
// 	crsfTxData[8] = 'F';                           // 0x05
// 	crsfTxData[9] = 'G';                           // 0x05
// 	crsfTxData[10] = 'H';                          // 0x06
// 	crsfTxData[11] = 'A';                          // 0x0B
// 	crsfTxData[12] = 'A';                          // 0x0C
// 	crsfTxData[13] = 'A';                          // 0x0D
// 	crsfTxData[14] = 'A';                          // 0x0E
// 	crsfTxData[15] = 'B';
// 	crsfTxData[16] = 'C'; // 0x10
// 	crsfTxData[17] = 'D';
// 	crsfTxData[18] = '\0'; // 0x12
// 	crsfTxData[19] = crsf_crc8(&crsfTxData[2], CRSF_FLIGHT_MODE_FRAME_LENGTH - 1);
// 	// crsfTxData[20] =
// 	// crsfTxData[21] = 'A'; // 0x15
// 	// crsfTxData[22] = '\0'; // 0x16
// 	// crsfTxData[23] =
// 	// crsfTxData[24] = 'A'; // 0x18
// 	// crsfTxData[25] = 'A'; // 0x19
// 	// crsfTxData[26] = 'A'; // 0x1A
// 	// crsfTxData[27] = 'A'; // 0x1B
// 	// crsfTxData[28] = 'A'; // 0x1C
// 	// crsfTxData[29] = 'A'; // 0x1D
// 	// crsfTxData[30] = 'A'; // 0x1E
// 	// crsfTxData[31] = 'A'; // 0x1F
// 	// crsfTxData[32] = 'A'; // 0x20
// 	// crsfTxData[33] = 'A'; // 0x21
// 	// crsfTxData[34] = 'A'; // 0x22
// 	// crsfTxData[35] = 'A'; // 0x23
// 	// crsfTxData[36] = 'A'; // 0x24
// 	// crsfTxData[37] = 'A'; // 0x25
// 	// crsfTxData[38] = 'A'; // 0x26
// 	// crsfTxData[39] = 'A'; // 0x27
// 	// crsfTxData[40] = 'A'; // 0x28
// 	// crsfTxData[41] = 'A'; // 0x29
// 	// crsfTxData[42] = 'A'; // 0x2A
// 	// crsfTxData[43] = 'A'; // 0x2B
// 	// crsfTxData[44] = 'A'; // 0x2C
// 	// crsfTxData[45] = 'A'; // 0x2D
// 	// crsfTxData[46] = 'A'; // 0x2E
// 	// crsfTxData[47] = 'A'; // 0x2F
// 	// crsfTxData[48] = 'A'; // 0x30
// 	// crsfTxData[49] = 'A'; // 0x31
// 	// crsfTxData[50] = 'A'; // 0x32
// 	// crsfTxData[51] = 'A'; // 0x33
// 	// crsfTxData[52] = 'A'; // 0x34
// 	// crsfTxData[53] = 'A'; // 0x35
// 	// crsfTxData[54] = 'A'; // 0x36
// 	// crsfTxData[55] = 'A'; // 0x37
// 	// crsfTxData[56] = 'A'; // 0x38
// 	// crsfTxData[57] = 'A'; // 0x39
// 	// crsfTxData[58] = 'A'; // 0x3A
// 	// crsfTxData[59] = 'A'; // 0x3B
// 	// crsfTxData[60] = 'A'; // 0x3C
// 	// crsfTxData[61] = 'A'; // 0x3D
// 	// crsfTxData[62] = '\0'; // 0x3E
// 	// crsfTxData[63] = crsf_crc8(&crsfTxData[2], CRSF_FLIGHT_MODE_FRAME_LENGTH - 1); // 0x54

// 	serialPort->write(crsfTxData, CRSF_FLIGHT_MODE_FRAME_LENGTH + 2);
// }