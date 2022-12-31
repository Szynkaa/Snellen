#include "systick.h"

#include "LPC17xx.h"


extern volatile int eint1DeadTime;
extern volatile int eint2DeadTime;
volatile uint32_t sysTickTimer;

void initializeSysTick(const uint32_t secondsNumerator, const uint32_t secondsDenominator) {
    SysTick->CTRL = 0b000;
    SysTick->LOAD = SystemCoreClock * secondsNumerator / secondsDenominator - 1;
    SysTick->CTRL = 0b111;
}

void SysTick_Handler() {
    if (eint1DeadTime > 0) {
        --eint1DeadTime;
    }
    sysTickTimer++;
}

void waitSysTick(const uint32_t timeUnit) {
    sysTickTimer = 0;
    while (sysTickTimer < timeUnit);
}
