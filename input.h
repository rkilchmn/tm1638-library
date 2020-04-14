#ifndef INPUT_H
#define INPUT_H
#endif

#include "TM1638GCA.h"
#include "Keypad.h" 

#define INPUT_ABBORT_CHAR '#'
#define INPUT_CONFIRM_CHAR '*'

String inputKeypad(TM1638GCA displayKeyModule, Keypad keyPad, const String label, uint8_t minLength, uint8_t maxLength, bool allowAlphaNumeric, String edit = "");

// selections[0] is label; returns selection 1-numSelections; return 0 is abort/not selected
uint8_t selectKeypad(TM1638GCA displayKeyModule, Keypad keyPad, String selections[], uint8_t numSelections, uint8_t selected = 0);
