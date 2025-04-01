#pragma once

#include <Arduino.h>

// ELRS
/*
 * CRSF protocol
 *
 * CRSF protocol uses a single wire half duplex uart connection.
 * The master sends one frame every 4ms and the slave replies between two frames from the master.
 *
 * 420000 baud
 * not inverted
 * 8 Bit
 * 1 Stop bit
 * Big endian
 * 420000 bit/s = 46667 byte/s (including stop bit) = 21.43us per byte
 * Max frame size is 64 bytes
 * A 64 byte frame plus 1 sync byte can be transmitted in 1393 microseconds.
 *
 * CRSF_TIME_NEEDED_PER_FRAME_US is set conservatively at 1500 microseconds
 *
 * Every frame has the structure:
 * <Device address><Frame length><Type><Payload><CRC>
 *
 * Device address: (uint8_t) or sync byte????
 * Frame length:   length in bytes including length Type (uint8_t)
 * Type:           (uint8_t)
 * Payload
 * CRC:            (uint8_t)
 *
 */

// Sync Byte
// #define CRSF_SYNC_BYTE 0xEC
#define CRSF_SYNC_BYTE 0xC8

// Device Address
#define CRSF_ADDRESS_MODULE            0xEE // Crossfire transmitter
#define CRSF_ADDRESS_RADIO             0xEA // Radio Transmitter
#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xC8 // Flight Controler
#define CRSF_ADDRESS_CRSF_RECEIVER     0xEC // Receiver

// Frame Length
#define CRSF_GPS_FRAME_LENGTH         17 // length of (type + payload (15) + crc)
#define CRSF_BATTSTATS_FRAME_LENGTH   10 // length of (type + payload (8) + crc)
#define CRSF_LINKSTATS_FRAME_LENGTH   12 // length of (type + payload (10) + crc)
#define CRSF_RCCHANNEL_FRAME_LENGTH   24 // length of (type + payload (22) + crc)
#define CRSF_ATTITUDE_FRAME_LENGTH    8  // length of (type + payload (6) + crc)
#define CRSF_FLIGHT_MODE_MAX_FRAME_LENGTH 16 // length of (type + payload (13) + '\0' + crc)
// #define CRSF_MSP_RESP_FRAME_LENGTH    62

// Frame Type
#define CRSF_FRAMETYPE_GPS                0x02
#define CRSF_FRAMETYPE_BATTERY_SENSOR     0x08
#define CRSF_FRAMETYPE_LINK_STATISTICS    0x14
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_FRAMETYPE_ATTITUDE           0x1E
#define CRSF_FRAMETYPE_FLIGHT_MODE        0x21
// #define CRSF_FRAMETYPE_MSP_RESP           0x7B // reply with 58 byte chunked binary
#define CRSF_FRAMETYPE_DEVICE_INFO 0x29

// Define channel input limit
#define CRSF_CHANNEL_MIN 172
#define CRSF_CHANNEL_MID 991
#define CRSF_CHANNEL_MAX 1810

// internal crsf variables
#define CRSF_MAX_CHANNEL              16
#define CRSF_MAX_RX_PACKET_SIZE       26                                // max is RC CHANNEL TYPE
#define CRSF_MAX_TX_PACKET_SIZE       64                                // max is CRSF_FRAMETYPE_MSP_RESP
#define CRSF_LINKSTATS_PAYLOAD_LENGTH (CRSF_LINKSTATS_FRAME_LENGTH - 2) // length of payload
#define CRSF_RCCHANNEL_PAYLOAD_LENGTH (CRSF_RCCHANNEL_FRAME_LENGTH - 2) // length of payload
#define CRSF_CONNECTION_TIMEOUT 200
#define CRSF_SERIAL_TIMEOUT 10

class CRSF
{
private:
	/* variables */
	HardwareSerial* serialPort;

	uint8_t crsfRxData[CRSF_MAX_RX_PACKET_SIZE];
	uint8_t crsfTxData[CRSF_MAX_TX_PACKET_SIZE];

	unsigned long crsfLinkStatsLastReceived;
	unsigned long crsfRCChannelLastReceived;
	bool txDisconnected;

	/*
	 * 0x16 Channels Value
	 * Payload:
	 * 11 bits ch0 - ch15
	 */
	typedef struct
	{
		// 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
		unsigned int chan0 : 11;
		unsigned int chan1 : 11;
		unsigned int chan2 : 11;
		unsigned int chan3 : 11;
		unsigned int chan4 : 11;
		unsigned int chan5 : 11;
		unsigned int chan6 : 11;
		unsigned int chan7 : 11;
		unsigned int chan8 : 11;
		unsigned int chan9 : 11;
		unsigned int chan10 : 11;
		unsigned int chan11 : 11;
		unsigned int chan12 : 11;
		unsigned int chan13 : 11;
		unsigned int chan14 : 11;
		unsigned int chan15 : 11;
	} __attribute__((__packed__)) crsfPayloadRcChannelsPacked_s;

	/*
	 * 0x14 Link statistics
	 * Payload:
	 * uint8_t Uplink RSSI Ant. 1 ( dBm * -1 )
	 * uint8_t Uplink RSSI Ant. 2 ( dBm * -1 )
	 * uint8_t Uplink Package success rate / Link quality ( % )
	 * int8_t Uplink SNR ( db )
	 * uint8_t Diversity active antenna ( ant. 1 = 0, ant. 2 = 1)
	 * uint8_t RF Mode ( enum 4fps = 0 , 50fps, 150hz)
	 * uint8_t Uplink TX Power ( 0mW = 0, 10mW, 25 mW, 100 mW, 500 mW, 1000 mW, 2000mW )
	 * uint8_t Downlink RSSI ( dBm * -1 )
	 * uint8_t Downlink package success rate / Link quality ( % )
	 * int8_t Downlink SNR ( db )
	 * Uplink is the connection from the ground to the ASV and downlink the opposite direction.
	 */

	typedef struct
	{
		uint8_t uplink_RSSI_1;
		uint8_t uplink_RSSI_2;
		uint8_t uplink_Link_quality;
		int8_t uplink_SNR;
		uint8_t active_antenna;
		uint8_t rf_Mode;
		uint8_t uplink_TX_Power; // not implemented in CRSF https://www.expresslrs.org/faq/
		uint8_t downlink_RSSI;
		uint8_t downlink_Link_quality;
		int8_t downlink_SNR;
	} __attribute__((__packed__)) crsfPayloadLinkstatistics_s;

	crsfPayloadLinkstatistics_s crsfLinkStats;
	uint16_t crsfChannels[CRSF_MAX_CHANNEL];

	/* functions */
	uint8_t crsf_crc8(const uint8_t* ptr, uint8_t len) const;

	void updateChannels();
	void updateLinkStats();

public:
	void begin(HardwareSerial* port, unsigned long baud = 420000);
	int serialBytesAvailable() const;
	void readPacket();
	uint16_t getChannel(uint8_t channel) const;
	int8_t getRxRSSI1() const;
	int8_t getRxRSSI2() const;
	int8_t getRxRSSI() const;
	int8_t getTxRSSI() const;
	uint8_t getActAnt() const;
	uint8_t getRFMode() const;
	uint8_t getRxLinkQuality() const;
	uint8_t getTxLinkQuality() const;
	int8_t getRxSNR() const;
	int8_t getTxSNR() const;
	bool isConnected();
	unsigned long getLinkStatsLastReceived() const;
	unsigned long getRCChannelLastReceived() const;
	void writeGPSPacket(int32_t latitude, int32_t longitude, uint16_t groundSpeed, uint16_t gpsHeading, uint16_t altitude, uint8_t satUsed);
	void writeBatteryPacket(uint16_t batt_voltage, uint16_t batt_current, uint32_t fuel, uint8_t battPercent);
    void writeAttitudePacket(uint16_t pitch, uint16_t roll, uint16_t yaw);
    void writeFlightModePacket(char* p_charArray, uint8_t msglen);
    void crsfFrameDeviceInfo(char* p_charArray, uint8_t msglen);
    void sendGarbaage();
    void crsfSendMspResponse();
};
