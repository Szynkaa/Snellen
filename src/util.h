#ifndef _UTIL_
#define _UTIL_

#include <stdint.h>

enum KeypadDigitReturnCodes {
    CODE_BACKSPACE = -3,
    CODE_ENTER = -2,
    CODE_IGNORE = -1
};

uint16_t swapEndianness(uint16_t x);
int keypadCodeToDigit(uint16_t code);
int min(const int a, const int b);
int max(const int a, const int b);

#endif // _UTIL_
