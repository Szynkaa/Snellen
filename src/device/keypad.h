#ifndef _KEYPAD_
#define _KEYPAD_

#include <stdbool.h>

extern volatile bool keypadPendingRead;

void initializeKeypad();
int readKeypad();
void clearKeypadPendingRead();
void checkKeypadInterrupt();

#endif // _KEYPAD_
