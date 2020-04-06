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

// https://github.com/rkilchmn/Keypad
#include <TM1638GCA.h>
// display map: maps digit (leftmost is index 0) to the connected KS1-8 pin
byte DISPLAYMAP[] = {8, 7, 6, 5, 4, 3};
// define a module on data pin 8, clock pin 9 and strobe pin 7
TM1638GCA displayKeypad(D6, D7, D8, DISPLAYMAP, sizeof(DISPLAYMAP));

#include "Keypad.h"         // tm1638-library provides drop
const byte KEYPAD_ROWS = 4; //four rows
const byte KEYPAD_COLS = 3; //three columns
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

Keypad kpd(makeKeymap(keys), displayKeypad, KEYPAD_ROWS, KEYPAD_COLS);

unsigned long loopCount;
unsigned long startTime;
String msg;

String scrollText = "This is a long text for scrolling";

byte scrollPos = 0;
unsigned long previousMillis = 0;

void setup()
{
    Serial.begin(115200);
    loopCount = 0;
    startTime = millis();
    msg = "";
}

void loop()
{
    loopCount++;
    if ((millis() - startTime) > 5000)
    {
        Serial.print("Average loops per second = ");
        Serial.println(loopCount / 5);
        startTime = millis();
        loopCount = 0;
    }

    char key = kpd.getKey();
    if (key)
    {
        KeyState state = kpd.getState();
        if (state) // Only find keys that have changed state.
        {
            switch (state)
            { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
            case PRESSED:
                msg = " PRESSED.";
                break;
            case HOLD:
                msg = " HOLD.";
                break;
            case RELEASED:
                msg = " RELEASED.";
                break;
            case IDLE:
                msg = " IDLE.";
            }
            Serial.print("Key ");
            Serial.print(key);
            Serial.println(msg);
        }
    }

    // // Fills kpd.key[ ] array with up-to 10 active keys.
    // // Returns true if there are ANY active keys.
    // if (kpd.getKeys())
    // {
    //     for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    //     {
    //         if (kpd.key[i].stateChanged) // Only find keys that have changed state.
    //         {
    //             switch (kpd.key[i].kstate)
    //             { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
    //             case PRESSED:
    //                 msg = " PRESSED.";
    //                 break;
    //             case HOLD:
    //                 msg = " HOLD.";
    //                 break;
    //             case RELEASED:
    //                 msg = " RELEASED.";
    //                 break;
    //             case IDLE:
    //                 msg = " IDLE.";
    //             }
    //             Serial.print("Key ");
    //             Serial.print(kpd.key[i].kchar);
    //             Serial.println(msg);
    //         }
    //     }
    // }

    yield();
    delay(50);

    displayKeypad.scrollDisplayToString( scrollText, &scrollPos, &previousMillis);
}
