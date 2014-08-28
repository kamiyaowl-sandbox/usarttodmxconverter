/*
* USARTtoDMXConverter.ino
*
* Created: 8/26/2014 11:36:27 PM
* Author: kamiya
*/

#include <SoftwareSerial.h>

#define DMXCHANNELS		128
#define SET_ADDR_HIGH	0x0
#define SET_ADDR_LOW	0x1
#define SET_DATA_HIGH	0x2
#define SET_DATA_LOW	0x3

#define DATA_CLEAR		0xc
#define SEND_DISABLE	0xd
#define SEND_ENABLE		0xe

uint8_t dmxData[DMXCHANNELS] = {};

#define DMX_TX 1
#define DMX_RX 0

#define PC_TX 9
#define PC_RX 8

SoftwareSerial pcSerial(PC_TX,PC_RX);
void setup()
{
	pcSerial.begin(19200);//Set Baud
}


void loop()
{
	/* Receive Data From PC */
	static uint8_t addrBuf = 0x0,dataBuf = 0x0;
	static uint8_t previousInstruction = 0x0;
	static uint8_t isSend = 0x1;
	if(pcSerial.available()){
		uint8_t received = pcSerial.read();
		uint8_t instruction = (received >> 4) & 0xf;
		uint8_t data = received & 0xf;
		switch(instruction){
			/* Main */
			case SET_ADDR_HIGH:
				addrBuf = 0x0;
				dataBuf = 0x0;
				addrBuf |= data << 4;
				previousInstruction = SET_ADDR_HIGH;
				break;
			case SET_ADDR_LOW:
				if(previousInstruction != SET_ADDR_HIGH) break;
				addrBuf |= data & 0xf;
				previousInstruction = SET_ADDR_LOW;
				break;
			case SET_DATA_HIGH:
				if(previousInstruction != SET_ADDR_LOW) break;
				dataBuf |= data << 4;
				previousInstruction = SET_DATA_HIGH;
				break;
			case SET_DATA_LOW:
				if(previousInstruction != SET_DATA_HIGH) break;
				dataBuf |= data & 0xf;
				//Apply
				if(addrBuf < DMXCHANNELS) dmxData[addrBuf] = dataBuf;
				previousInstruction = SET_DATA_LOW;
				break;
			/* Instruction */
			case DATA_CLEAR:
				for(uint8_t i = 0 ; i < DMXCHANNELS ; ++i){
					dmxData[i] = 0x0;
				}
			break;
				case SEND_DISABLE:
				isSend = 0x0;
				break;
			case SEND_ENABLE:
				isSend = 0x1;
				break;
		}
	}
	/* Send DMX Data */
	if(isSend){
		pinMode(DMX_TX,OUTPUT);
		digitalWrite(DMX_TX,0);//Break
		delayMicroseconds(88);
		digitalWrite(DMX_TX,1);//MAB
		delayMicroseconds(8);
		Serial.begin(250000,SERIAL_8N2);
		Serial.write(0x0);//Start Code
		for(uint16_t i = 0 ; i < DMXCHANNELS ; ++i){
			Serial.write(dmxData[i]);//Ch Data
		}
		Serial.end();
	}
}