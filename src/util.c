#include "util.h"

inline uint16_t swapEndianness(uint16_t x) {
    return (x << 8) | (x >> 8);
}

int keypadCodeToDigit(uint16_t code) {
    if (code == 0) {
        return -3;
    }

    const int mapping[16] = {
        -1, -3, -3, 0,
        -2, 9, 8, 7,
        -3, 6, 5, 4,
        -3, 3, 2, 1,
    };

    int index = 0;
    while (!(code & 1)) {
        code >>= 1;
        index++;
    }

    return mapping[index];
}
