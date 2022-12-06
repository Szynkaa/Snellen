#include "LPC17xx.h"

#include "util.h"


void initializeSysTick(uint32_t secondsNumerator, uint32_t secondsDenominator) {
	SysTick->CTRL = 0b000;
	SysTick->LOAD = SystemCoreClock * secondsNumerator / secondsDenominator - 1;
	SysTick->CTRL = 0b111;
}

inline uint16_t swapEndianness(uint16_t x) {
	return (x << 8) | (x >> 8);
}
