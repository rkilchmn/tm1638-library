/*
TM1638GCA.h - Library for TM1638 for Generic Common Anode schematic.

Copyright (C) 2011 Ricardo Batista <rjbatista at gmail dot com>
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

#ifndef TM1638GCA_h
#define TM1638GCA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "TM16XX.h"
#include "TM16XXFonts.h"

struct keyPressed_s {
  byte Kn;  // K1-3
  byte KSn; // KS1-8
};


class TM1638GCA : public TM16XX
{
  public:
    /** Instantiate a tm1638 module specifying the display state, the starting intensity (0-7) data, clock and stobe pins.
     *  max 8 displays supported in Common Annode Design
     *  display map: maps digit (leftmost is index 0) to the connected KS1-8 pin
     */
    TM1638GCA(byte dataPin, byte clockPin, byte strobePin, byte *displayMap, byte displays = 8, boolean activateDisplay = true, byte intensity = 7);

    /** Set the display to a unsigned hexadecimal number (with or without leading zeros) */
    void setDisplayToHexNumber(unsigned long number, byte dots, boolean leadingZeros = true,
		const byte numberFont[] = FONT_DEFAULT);
    /** Set the display to a unsigned decimal number (with or without leading zeros) */
    void setDisplayToDecNumber(unsigned long number, byte dots, boolean leadingZeros = true,
		const byte numberFont[] = FONT_DEFAULT);
    /** Set the display to a signed decimal number (with or without leading zeros) */
    void setDisplayToSignedDecNumber(signed long number, byte dots, boolean leadingZeros = true,
		const byte numberFont[] = FONT_DEFAULT);
    /** Set the display to a unsigned binary number */
    void setDisplayToBinNumber(byte number, byte dots,
		const byte numberFont[] = NUMBER_FONT);

	/** Clear the display */
	virtual void clearDisplay();
    /** Set the display to the String (defaults to built in font) - pos is ignored in common anode */
	virtual void setDisplayToString(const char* string, const word dots = 0, const byte pos = 0,
		const byte font[] = FONT_DEFAULT);
    /** Set the display to the String (defaults to built in font) - pos is ignored in common anode */
	virtual void setDisplayToString(String string, const word dots = 0, const byte pos = 0,
		const byte font[] = FONT_DEFAULT);

    /** Returns the pressed buttons as a bit set (left to right). */
  virtual word getButtons();

    /** Returns the key scans up to maxScans into a 4 byte array and returns number of bytes (0 to max 4)
     * b0 b1 b2 b3  b4 b5 b6 b7
     * K3 K2 K2 LSB K3 K2 K1 MSB
     * MSB and LSB encodes the received byte number ie 00=0,01=1,10=2,11=3
     * byte 0: K3-K1 for KS1 (b0-b2) and K3-K1 for KS1 (b4-b6)
     * byte 1: K3-K1 for KS3 (b0-b2) and K3-K1 for KS4 (b4-b6) 
     * byte 2: K3-K1 for KS5 (b0-b2) and K3-K1 for KS6 (b4-b6)
     * byte 3: K3-K1 for KS7 (b0-b2) and K3-K1 for KS7 (b4-b6) 
     */
    virtual byte getKeyScans(byte *keyScanResult, byte maxScanResults);

    // decode scan results into key pressed by its Kn/KSn scan matrix column and row
    virtual byte decodeKeyPressed(byte *keyScanResult, byte numScansResult, keyPressed_s *keyPressed, byte maxKeyPressed);

    virtual void scrollDisplayToString(String scrollText, byte *scrollPos, unsigned long *previousMillis, unsigned long scrollIntervalMillis = 250);

    virtual int getDisplayDigits() { return displays; }

  protected:
	/** Set the display to the values (left to right)
   */
    virtual void setDisplay(const byte values[], unsigned int length = 8);

  private:
	// unsupported in common anode design
    virtual void setDisplayDigit(byte digit, byte pos, boolean dot, const byte numberFont[] = NUMBER_FONT) { setDisplayToError(); };
	// unsupported in common anode design
    virtual void clearDisplayDigit(byte pos, boolean dot) { setDisplayToError(); };
	// unsupported in common anode design
	virtual void sendChar(byte pos, byte data, boolean dot) { setDisplayToError(); }

  byte _displayMap[8];
};

#endif
