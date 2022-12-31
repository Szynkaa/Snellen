#include "keys.h"

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "device/console.h"


volatile int eint0DeadTime = 0;
volatile int eint1DeadTime = 0;

void EINT0_IRQHandler() {
    LPC_SC->EXTINT = 1 << 0;

    if (eint0DeadTime > 0) {
        return;
    }

    printChar('0');
    eint0DeadTime = 200;
}

void EINT1_IRQHandler() {
    LPC_SC->EXTINT = 1 << 1;

    if (eint1DeadTime > 0) {
        return;
    }

    printChar('1');
    eint1DeadTime = 200;
}

void initializeKeys() {
    // Key 0
    PIN_Configure(2, 10, 0b01, 0, 0);

    LPC_SC->EXTMODE = (LPC_SC->EXTMODE & 0b1111) | 1 << 0;
    NVIC_EnableIRQ(EINT0_IRQn);

    // Key 1
    PIN_Configure(2, 11, 0b01, 0, 0);

    LPC_SC->EXTMODE = (LPC_SC->EXTMODE & 0b1111) | 1 << 1;
    NVIC_EnableIRQ(EINT1_IRQn);
}