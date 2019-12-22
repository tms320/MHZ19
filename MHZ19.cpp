/*
 * Designed to work with Winsen MH-Z19B CO2 sensor
 * - https://www.winsen-sensor.com/products/ndir-co2-sensor/mh-z19b.html
 * According to:
 * - https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf
 *
 * This library implements methods for working with sensor via UART.
 *
 * Written by Kostiantyn Levytskyi <le.konstantos@gmail.com>
 *
 * MIT License
 *
 */

#include <MHZ19.h>


MHZ19::MHZ19()
{
	_isReady = false;
	_uart = NULL;
}


MHZ19::MHZ19(Stream* uart) : MHZ19()
{
	_uart = uart;
}


void MHZ19::setUART(Stream* uart)
{
	_uart = uart;
}


bool MHZ19::isReady()
{
	if (!_isReady)
	{
		if ((_uart != NULL) && ((int64_t)millis() > PREHEAT_TIME))
		{
			_isReady = true;
		}
	}
	return _isReady;
}


bool MHZ19::setRange2000()
{
	static byte cmdSetRange2000[] = { 0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x8F };
	return sendCmd(cmdSetRange2000);
}


bool MHZ19::setRange5000()
{
	static byte cmdSetRange5000[] = { 0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB };
	return sendCmd(cmdSetRange5000);
}


bool MHZ19::setRange10000()
{
	static byte cmdSetRange10000[] = { 0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x27, 0x10, 0x2F };
	return sendCmd(cmdSetRange10000);
}


bool MHZ19::enableSelfCalibration()
{
	static byte cmdEnableABC[] = { 0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6 };
	return sendCmd(cmdEnableABC);
}


bool MHZ19::disableSelfCalibration()
{
	static byte cmdDisableABC[] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86 };
	return sendCmd(cmdDisableABC);
}


bool MHZ19::calibrateZeroPoint()
{
	static byte cmdCalibrateZero[] = { 0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78 };
	return sendCmd(cmdCalibrateZero);
}


bool MHZ19::calibrateSpanPoint(int span)
{
	byte spanHigh = span / 256;
	byte spanLow = span & 0xFA;	// span % 256;
	byte cmdCalibrateSpan[] = { 0xFF, 0x01, 0x88, spanHigh, spanLow, 0x00, 0x00, 0x00, 0x00 };
	cmdCalibrateSpan[8] = calcCRC(cmdCalibrateSpan);
	return sendCmd(cmdCalibrateSpan);
}


int MHZ19::getCO2()
{
	static byte cmdRead[] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
	if (_uart == NULL) return 0;
	int64_t now = (int64_t)millis();
	if (now > PREHEAT_TIME)
	{
		if (sendCmd(cmdRead))
		{
			return (256 * (int)_response[2]) + (int)_response[3];
		}
		return 0;
	}
	return (int)((now - PREHEAT_TIME) / 1000);
}


byte MHZ19::calcCRC(byte data[9])
{
	byte crc = 0;
	for (int i = 1; i < 8; i++)
	{
		crc += data[i];
	}
	return 0xFF - crc + 1;
}

bool MHZ19::sendCmd(byte cmd[9])
{
	if (_uart == NULL) return false;

	_uart->write(cmd, 9);
	int n = 0;
	int64_t startTime = (int64_t)millis();
	do
	{
		int m = (int)_uart->readBytes(_response + n, 9 - n);
		if (m > 0) n += m;
		if ((int64_t)millis() - startTime >= 100)
		{
			//Serial.println("MHZ19::sendCmd: Timeout");
			return false;
		}
	} while (n < 9);
	//Serial.print("MHZ19::sendCmd: "); for (int i = 0; i < n; i++) Serial.printf("%02X ", _response[i]); Serial.println();
	byte crc = calcCRC(_response);
	return (_response[0] == 0xFF) && (_response[1] == cmd[2]) && (_response[8] == crc);
}
