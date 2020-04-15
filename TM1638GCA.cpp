/*
TM1638GCA.cpp - Library for TM1638 for Generic Common Anode schematic.

Copyright (C) 2011 Ricardo Batista (rjbatista <at> gmail <dot> com)
Copyright (C) 2020 Roger Kilchenmann (roger <at> kilchenmann <dot> net)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "TM1638GCA.h"
#include "string.h"

TM1638GCA::TM1638GCA(byte dataPin, byte clockPin, byte strobePin, byte *displayMap, byte displays, boolean activateDisplay, byte intensity)
	: TM16XX(dataPin, clockPin, strobePin, displays, activateDisplay, intensity)
{
	// copy display max
	memcpy(_displayMap, displayMap, displays);
}

void TM1638GCA::setDisplay(const byte values[], unsigned int length)
{
	// common anode logic always needs to send all 8 bytes because segments are transposed
	const byte CA_LOGIC = 8;

	for (int i = 0; i < CA_LOGIC; i++)
	{
		int val = 0;

		for (int j = 0; j < length; j++)
		{
			// SEG8-1 is mapped to B7-B1 -> shift left accordingly defined by displayMap
			val |= ((values[j] >> i) & 1) << (_displayMap[j] - 1);
		}

		sendData(i << 1, val);
	}
}

void TM1638GCA::clearDisplay()
{
	setDisplay(NULL, 0);
}

void TM1638GCA::setDisplayToString(const char *string, const word dots, const byte ignored, const byte font[])
{
	byte values[displays];
	boolean done = false;

	memset(values, 0, displays * sizeof(byte));

	for (int i = 0; i < displays; i++)
	{
		if (!done && string[i] != '\0')
		{
			values[i] = font[string[i] - 32] | (((dots >> (CA_LOGIC - _displayMap[i])) & 1) << 7);
		}
		else
		{
			done = true;
			values[i] = (((dots >> (displays - i - 1)) & 1) << 7);
		}
	}

	setDisplay(values, displays);
}

void TM1638GCA::setDisplayToString(String string, const word dots, const byte ignored, const byte font[])
{
	setDisplayToString( string.c_str(), dots, ignored, font);
}

void TM1638GCA::setDisplayToBinNumber(byte number, byte dots, const byte numberFont[])
{
	byte values[displays];

	memset(values, 0, displays * sizeof(byte));

	for (int i = 0; i < displays; i++)
	{
		values[i] = numberFont[(number >> (displays - i - 1)) & 1] | (((dots >> (displays - i - 1)) & 1) << 7);
	}

	setDisplay(values, displays);
}

void TM1638GCA::setDisplayToHexNumber(unsigned long number, byte dots, boolean leadingZeros,
									  const byte numberFont[])
{
	char values[displays + 1];

	snprintf(values, displays + 1, leadingZeros ? "%08X" : "%X", number); // ignores display size

	setDisplayToString(values, dots, 0, numberFont);
}

void TM1638GCA::setDisplayToDecNumber(unsigned long number, byte dots, boolean leadingZeros,
									  const byte numberFont[])
{
	char values[displays + 1];

	snprintf(values, displays + 1, leadingZeros ? "%08ld" : "%ld", number); // ignores display size

	setDisplayToString(values, dots, 0, numberFont);
}

void TM1638GCA::setDisplayToSignedDecNumber(signed long number, byte dots, boolean leadingZeros,
											const byte numberFont[])
{
	char values[displays + 1];

	snprintf(values, displays + 1, leadingZeros ? "%08d" : "%d", number); // ignores display size

	setDisplayToString(values, dots, 0, numberFont);
}

word TM1638GCA::getButtons(void)
{
	word keys = 0;

	digitalWrite(strobePin, LOW);
	send(0x42); // B01000010 Read the key scan data
	for (int i = 0; i < 4; i++)
	{
		byte rec = receive();

		rec = (((rec & B1000000) >> 3 | (rec & B100)) >> 2) | (rec & B100000) | (rec & B10) << 3;

		keys |= ((rec & 0x000F) << (i << 1)) | (((rec & 0x00F0) << 4) << (i << 1));
	}
	digitalWrite(strobePin, HIGH);

	return keys;
}

byte TM1638GCA::getKeyScans(byte *keyScanResult, byte maxScanResults)
{
	byte numScans = 0;

	digitalWrite(strobePin, LOW);
	send(0x42); // B01000010 Read the key scan data
	for (byte i = 0; i < 4; i++)
	{
		byte rec = receive();
		if ((rec > 0) && (numScans < maxScanResults))
		{
			switch (i)
			{ // encode the received byte number into bit 3 and bit 7
			case 1:
				rec |= B00001000; // set bit 3 = 1
				break;
			case 2:
				rec |= B10000000; // set bit 7 = 2
				break;
			case 3:
				rec |= B10001000; // set bit 3 and 7 = 3
				break;
			}
			*keyScanResult = rec;
			keyScanResult++;
			numScans++;
		}
	}
	digitalWrite(strobePin, HIGH);

	return numScans;
}

byte TM1638GCA::decodeKeyPressed(byte *keyScanResult, byte numScansResult, keyPressed_s *keyPressed, byte maxKeyPressed)
{
	byte numKeyPressed = 0;
	for (byte i = 0; i < numScansResult; i++)
	{
		byte scanResult = *keyScanResult;
		byte posByteReceived = 0;

		// the received byte number position as a 2 bit number is encoded in bit 7 (MSB) and bit 3 (LSB)
		if (scanResult & (1 << 7))
		{							   // true if bit 7 set {
			posByteReceived ^= 1 << 1; // set bit 1 (MSB)
			scanResult &= ~(1 << 7);   // clear bit 7
		}
		if (scanResult & (1 << 3))
		{							   // true if bit 3 set {
			posByteReceived ^= 1 << 0; // set bit 0 (LSB)
			scanResult &= ~(1 << 3);   // clear bit 7
		}

		for (byte j = 0; j < 2; j++)
		{ // process for each nibble starting with lower nibble
			for (byte k = 0; k < 3; k++)
			{
				if (scanResult & (1 << k) && (numKeyPressed < maxKeyPressed))
				{ // bit k set 0:K3,1:K2,2:K3
					(*keyPressed).Kn = 3 - k;
					(*keyPressed).KSn = (posByteReceived * 2) + (j + 1); // depending on lower/higher nibble j
					keyPressed++;										 // move to next
					numKeyPressed++;									 // increase returned number
				}
			}
			scanResult = scanResult >> 4; // next round higher nibble shifted into lower nibbles
		}
		keyScanResult++;
	}

	return numKeyPressed;
}

void TM1638GCA::scrollDisplayToString(String scrollText, byte *scrollPos, unsigned long *previousMillis, unsigned long scrollIntervalMillis)
{
	unsigned long currentMillis = millis();

	if (currentMillis - *previousMillis >= scrollIntervalMillis)
	{
		// save the last time you blinked the LED
		*previousMillis = currentMillis;

		String display = "";

		// max display-1 blanks padded left
		for (byte i = 0; i < max(0, (int)(displays - *scrollPos - 1)); i++)
		{
			display += " ";
		}

		// part of scroll text visible
		byte startIndex = max(0, *scrollPos - displays + 1);
		display.concat(scrollText.substring(startIndex, startIndex + displays));

		// max display blanks padded right
		while (display.length() < displays)
		{
			display.concat(" ");
		}
		setDisplayToString(display);

		if (*scrollPos < (scrollText.length() + displays - 2))
		{
			*scrollPos += 1;
		}
		else
		{
			*scrollPos = 0;
		}
	}
}