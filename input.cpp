#include "input.h"

#define DEBUG

// use this to update
byte debugInputStatus(byte newStatus)
{
#ifdef DEBUG
  Serial.println("Input - Status:" + String(newStatus));
#endif

  return newStatus;
}

String inputKeypad(TM1638GCA displayKeyModule, Keypad keyPad, const String label, uint8_t minLength, uint8_t maxLength, bool allowAlphaNumeric, String edit)
{
#define INPUT_STATUS_LABEL 1      // display label
#define INPUT_STATUS_INPROGRESS 2 // awaiting further input
#define INPUT_STATUS_SUCCESS 3    // input minimum achieved
#define INPUT_STATUS_ABORT 4      // aborted

#define INPUT_KEY_CYCLE_MILLIS_MAX 1000
#define INPUT_KEY_CYCLE_ARRAY_SIZE 12
  String keyCycleArray[INPUT_KEY_CYCLE_ARRAY_SIZE] = {
      "1-.", "2abcABC", "3defDEF",
      "4ghiGHI", "5jklJKL", "6mnoMNO",
      "7pqrsPQRS", "8tuvTUV", "9wxyzWXYZ",
      "*", "0", "#"};

  // Test: 33 555 9999 307 777 222 77 839
  uint8_t status = debugInputStatus(INPUT_STATUS_LABEL);

  byte scrollPos = 0;
  unsigned long scrollMillis = 0;

  unsigned long currentMillis = 0;
  unsigned long keyMillis = 0;
  unsigned long lastKeyMillis = 0;

  char key = NO_KEY;
  char lastKey;

  displayKeyModule.clearDisplay();

  String input = edit;
  bool editMode = (input != "");

  do
  {
    currentMillis = millis();

    char newKey = keyPad.getKey();
    switch (keyPad.getState())
    {
    case RELEASED:

      lastKeyMillis = keyMillis;
      keyMillis = currentMillis;

      lastKey = key;
      key = newKey;

      break;
    default:
      newKey = NO_KEY;
    }

    if (status == INPUT_STATUS_LABEL)
    {
      switch (newKey)
      {
      case NO_KEY:
        displayKeyModule.scrollDisplayToString(label, &scrollPos, &scrollMillis);
        break;
      case INPUT_ABBORT_CHAR:
        status = INPUT_STATUS_ABORT;
        input = "";
        break;
      case INPUT_CONFIRM_CHAR:
        if (input.length() >= minLength)
          status = debugInputStatus(INPUT_STATUS_SUCCESS);
        break;
      default:
        displayKeyModule.clearDisplay();
        status = debugInputStatus(INPUT_STATUS_INPROGRESS);
      }
    }

    if (status == INPUT_STATUS_INPROGRESS)
    {
      switch (newKey)
      {
      case NO_KEY:
        break;
      case INPUT_CONFIRM_CHAR:
        if (input.length() >= minLength)
        {
          status = debugInputStatus(INPUT_STATUS_SUCCESS);
          break;
        }
      case INPUT_ABBORT_CHAR:
        if (input.length() > 0)
          input.remove(input.length() - 1);
        else
          status = debugInputStatus(INPUT_STATUS_ABORT);
        break;
      default:
        // in edit mode, first key is not recognized
        if (editMode)
          editMode = false;
        else if (allowAlphaNumeric && (newKey == lastKey) && (keyMillis - lastKeyMillis < INPUT_KEY_CYCLE_MILLIS_MAX))
        {
          char currentInput = input.charAt(input.length() - 1);
          for (int i = 0; i < INPUT_KEY_CYCLE_ARRAY_SIZE; ++i)
          {
            String keyMap = keyCycleArray[i];

            if (keyMap.startsWith(String(newKey)))
            { // first character
              int c = keyMap.indexOf(currentInput);
              if (c < keyMap.length())
                c++;
              else
                c = 0;
              currentInput = keyMap.charAt(c);

              break;
            }
          }
          input.remove(input.length() - 1); // remove last char
          input.concat(currentInput);       // add current
        }
        else
          input.concat(newKey); // input.length() starts with 1
      }

      // update display if there was a key event
      if (newKey != NO_KEY)
      {
        // show last displayKeyModule.getDisplayDigits() of entered intput (scroll to right)
        unsigned int start = max(0, int(input.length() - displayKeyModule.getDisplayDigits()));
        unsigned int end = start + min(int(input.length()), displayKeyModule.getDisplayDigits());
        String txt = input.substring(start, end);
        displayKeyModule.setDisplayToString(txt);
      }
    }

    // allow background tasks to be processed
    yield();
    delay(10); // or yield() could be called as well but not generally supported for all arduino platforms
  } while ((status != INPUT_STATUS_SUCCESS) && (status != INPUT_STATUS_ABORT));

  displayKeyModule.clearDisplay();
  return input;
}

// selections[0] is label; returns selection 1-numSelections; return 0 is abort/not selected
uint8_t selectKeypad(TM1638GCA displayKeyModule, Keypad keyPad, String selections[], uint8_t numSelections, uint8_t selected)
{

#define INPUT_LABEL_MAX_LENGTH 32
#define INPUT_STATUS_LABEL 1      // display label
#define INPUT_STATUS_INPROGRESS 2 // awaiting further input
#define INPUT_STATUS_SUCCESS 3    // input minimum achieved
#define INPUT_STATUS_ABORT 4      // aborted

  int8_t curSel = 0; // show label
  uint8_t status = debugInputStatus(INPUT_STATUS_LABEL);

  unsigned long previousMillis = 0;
  uint8_t scrollPos = 0;

  displayKeyModule.clearDisplay();

  bool showSelectedFirst = (selected != 0);

  Serial.println("curSel=" + String(curSel) + " numSelections=" + String(numSelections));

  do
  {
    displayKeyModule.scrollDisplayToString(selections[curSel], &scrollPos, &previousMillis);

    char key = keyPad.getKey();
    if (key)
    {
      KeyState state = keyPad.getState();
      if (state) // Only find keys that have changed state.
      {
        switch (state)
        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        case PRESSED:
          break;
        case HOLD:
          break;
        case RELEASED:
          switch (key)
          {
          case INPUT_ABBORT_CHAR:
            status = debugInputStatus(INPUT_STATUS_ABORT);
            break;
          case INPUT_CONFIRM_CHAR:
            if (curSel > 0)
            {
              status = debugInputStatus(INPUT_STATUS_SUCCESS);
            }
            break;
          default:
            status = debugInputStatus(INPUT_STATUS_INPROGRESS);

            if (showSelectedFirst)
            {
              Serial.println("showSelectFirst=" + String(selected));
              showSelectedFirst = false;
              curSel = selected;
            }
            else
            {
              // display next selection or start over with first selection
              if (numSelections > 0) // if numSelections is 0 it keeps showing label until abbort
              {
                if (curSel < numSelections)
                  curSel++;
                else
                  curSel = 1;
              }

              Serial.println(curSel);
            }

            // start scrolling next entry
            displayKeyModule.clearDisplay();
            scrollPos = 0;
            previousMillis = 0;
          }

          break;
        case IDLE:
          break;
        }
      }
    }

    delay(10);
    yield();

  } while ((status != INPUT_STATUS_SUCCESS) && (status != INPUT_STATUS_ABORT));
  displayKeyModule.clearDisplay();

  return curSel;
}
