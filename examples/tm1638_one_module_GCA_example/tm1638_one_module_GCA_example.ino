/*
Library examples for TM1638.

Copyright (C) 2011 Ricardo Batista <rjbatista at gmail dot com>

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

#include <TM1638GCA.h>

// display map: maps digit (leftmost is index 0) to the connected KS1-8 pin
byte displayMap[] = { 8,7,6,5,4,3 }; 

String scrollText = "This is a long text for scrolling";

byte scrollPos = 0;
unsigned long previousMillis = 0;


// define a module on data pin 8, clock pin 9 and strobe pin 7
TM1638GCA module(D6, D7, D8, displayMap, sizeof(displayMap));

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  byte scansResults[4];
 
  byte numScansResults = module.getKeyScans(scansResults, sizeof(scansResults));

  keyPressed_s keyPressed[8];
  byte numKeyPressed = module.decodeKeyPressed(scansResults, numScansResults, keyPressed, sizeof(keyPressed));

  //module.setDisplayToDecNumber(numKeyPressed, 0, false);

  for (byte i = 0; i < numKeyPressed; i++)
  { 
    Serial.println("Key");
    Serial.println(keyPressed[i].Kn);
    Serial.println(keyPressed[i].KSn);
  }

  module.scrollDisplayToString( scrollText, &scrollPos, &previousMillis);

  delay(100);
}
