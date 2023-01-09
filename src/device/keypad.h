#include <stdbool.h>

extern volatile bool keypadPendingRead;

void initializeKeypad();
int readKeypad();
void clearKeypadPendingRead();
void checkKeypadInterrupt();
