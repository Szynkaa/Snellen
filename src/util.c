#include "util.h"

inline uint16_t swapEndianness(uint16_t x) {
    return (x << 8) | (x >> 8);
}

int keypadCodeToDigit(uint16_t code) {
    if (code == 0) {
        return CODE_IGNORE;
    }

    static const int mapping[16] = {
        CODE_BACKSPACE, CODE_IGNORE, CODE_IGNORE, 0,
        CODE_ENTER,  9, 8, 7,
        CODE_IGNORE, 6, 5, 4,
        CODE_IGNORE, 3, 2, 1,
    };

    int index = 0;
    while (!(code & 1)) {
        code >>= 1;
        index++;
    }

    return mapping[index];
}
