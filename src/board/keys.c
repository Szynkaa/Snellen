#include <stdlib.h>

#include "keys.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "device/console.h"


volatile int eint0DeadTime = 0;
volatile int eint1DeadTime = 0;

void (*key0Callback)();
void (*key1Callback)();

void EINT0_IRQHandler() {
    LPC_SC->EXTINT = 1 << 0;

    if (eint0DeadTime > 0) {
        return;
    }

    if (key0Callback) {
        key0Callback();
    }
    
    eint0DeadTime = 200;
}

void EINT1_IRQHandler() {
    LPC_SC->EXTINT = 1 << 1;

    if (eint1DeadTime > 0) {
        return;
    }

    if (key1Callback) {
        key1Callback();
    }

    eint1DeadTime = 200;
}

void initializeKeys() {
    // Key 0
    PIN_Configure(2, 10, 0b01, 0, 0);

    LPC_SC->EXTMODE = (LPC_SC->EXTMODE & 0b1111) | 1 << 0;
    key0Callback = NULL;
    NVIC_EnableIRQ(EINT0_IRQn);

    // Key 1
    PIN_Configure(2, 11, 0b01, 0, 0);

    LPC_SC->EXTMODE = (LPC_SC->EXTMODE & 0b1111) | 1 << 1;
    key1Callback = NULL;
    NVIC_EnableIRQ(EINT1_IRQn);
}
