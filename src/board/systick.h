#ifndef _SYSTICK_
#define _SYSTICK_

#include <stdint.h>

void initializeSysTick(const uint32_t secondsNumerator, const uint32_t secondsDenominator);

// wait for approx timeUnit * secondsNumerator / secondsDenominator
void waitSysTick(const uint32_t timeUnit);

#endif // _SYSTICK_
