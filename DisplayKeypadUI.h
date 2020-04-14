#ifndef DISPLAYKEYPADUI_H
#define DISPLAYKEYPADUI_H

#include "TM1638GCA.h"
#include "Keypad.h" 

#define INPUT_ABBORT_CHAR '#'
#define INPUT_CONFIRM_CHAR '*'

class DisplayKeypadUI {
public:
	DisplayKeypadUI (TM1638GCA dispKpd, Keypad kpd) : _dispKpd( dispKpd), _kpd ( kpd) {}
    String enterText(const String label, uint8_t minLength, uint8_t maxLength, bool allowAlphaNumeric, String edit = "");
    // selections[0] is label; returns selection 1-numSelections; return 0 is abort/not selected
    uint8_t selectOptions(String selections[], uint8_t numSelections, uint8_t selected = 0);
private:
    byte debugInputStatus(byte newStatus);

    TM1638GCA _dispKpd;
    Keypad _kpd;
};

#endif
