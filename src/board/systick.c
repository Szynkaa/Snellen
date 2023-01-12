#include "systick.h"
#include "LPC17xx.h"

extern volatile int eint0DeadTime;
extern volatile int eint1DeadTime;
extern volatile int keypadDeadTime;
volatile uint32_t sysTickTimer;

void initializeSysTick(const uint32_t secondsNumerator, const uint32_t secondsDenominator) {
    SysTick->CTRL = 0b000;
    SysTick->LOAD = SystemCoreClock * secondsNumerator / secondsDenominator - 1;
    SysTick->CTRL = 0b111;
}

void SysTick_Handler() {
    if (eint0DeadTime > 0) {
        --eint0DeadTime;
    }

    if (eint1DeadTime > 0) {
        --eint1DeadTime;
    }

    if (keypadDeadTime > 0) {
        --keypadDeadTime;
    }

    sysTickTimer++;
}

void waitSysTick(const uint32_t timeUnit) {
    sysTickTimer = 0;
    while (sysTickTimer < timeUnit);
}
