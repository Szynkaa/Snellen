#include <stdbool.h>

extern volatile bool readLengthFromKeypad;

void initializeKeypad();
int readKeypad();
void checkKeypadInterrupt();
