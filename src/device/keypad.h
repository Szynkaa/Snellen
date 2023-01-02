#include <stdbool.h>

extern bool readLengthFromKeypad;

void initializeKeypad();
int readKeypad();
void checkKeypadInterrupt();
