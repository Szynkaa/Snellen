#include "random.h"

unsigned int randomState;

void randomSetSeed(int seed) {
    randomState = seed;
}

unsigned int randomNext() {
    randomState *= 1664525;
    randomState += 1013904223;
    return randomState;
}
