#include <stdint.h>


void initializeSysTick(const uint32_t secondsNumerator, const uint32_t secondsDenominator);

// wait for aprox timeUnit * secondsNumerator / secondsDenominator
void waitSysTick(const uint32_t timeUnit);
