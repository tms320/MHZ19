/*
 * Library to work with Winsen MH-Z19B CO2 sensor via UART
 * https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19b.html
 *
 * Written by Sergey Trofimov
 * https://github.com/tms320/MHZ19
 *
 * GNU General Public License v3.0
 */
#pragma once

#include <Arduino.h>

class MHZ19
{
public:
	MHZ19();
	MHZ19(Stream* uart);
	
	void setUART(Stream* uart);

	bool isReady();	// returns 'true' when preheat timed out (and uart is not NULL)

	bool setRange2000();
	bool setRange5000();
	bool setRange10000();

	bool enableSelfCalibration();
	bool disableSelfCalibration();
	
	bool calibrateZeroPoint();
	bool calibrateSpanPoint(int span);
	
	int getCO2();	// negative value is seconds left to preheat; 0 - error

private:
	static const int64_t PREHEAT_TIME = 180000;	// ms (3 minutes according datasheet)

	bool _isReady;
	Stream* _uart;
	byte _response[9];
	
	byte calcCRC(byte data[9]);
	bool sendCmd(byte cmd[9]);
};
