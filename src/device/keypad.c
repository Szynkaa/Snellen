#include <stddef.h>

#include "keypad.h"
#include "console.h"
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

volatile bool keypadPendingRead = false;

volatile int keypadDeadTime = 0;

#define INPUT_SIZE 4
static volatile LPC_GPIO_TypeDef* const inputPorts[INPUT_SIZE] = { LPC_GPIO0, LPC_GPIO0, LPC_GPIO1, LPC_GPIO0 };
static const uint8_t inputPins[INPUT_SIZE] = { 8, 9, 31, 6 };

#define OUTPUT_SIZE 4
static volatile LPC_GPIO_TypeDef* const outputPort = LPC_GPIO0;
static const int outputPins[4] = { 17, 18, 15, 16 };

void initializeKeypad() {
    // input
    PIN_Configure(0, 6, 0b00, 0b10, true); // 1 COL4
    PIN_Configure(1, 31, 0b00, 0b10, true); // 2 COL3
    PIN_Configure(0, 9, 0b00, 0b10, true); // 3 COL2
    PIN_Configure(0, 8, 0b00, 0b10, true); // 4 COL1

    // output
    PIN_Configure(0, 16, 0b00, 0b00, false); // 7 ROW4
    PIN_Configure(0, 15, 0b00, 0b00, false); // 8 ROW3
    PIN_Configure(0, 18, 0b00, 0b00, false); // 9 ROW2
    PIN_Configure(0, 17, 0b00, 0b00, false); // 10 ROW1

    LPC_GPIO0->FIODIR = (1 << 6) | (1 << 8) | (1 << 9);
    LPC_GPIO1->FIODIR = 1 << 31;

    for (int input_pin_index = 0; input_pin_index < INPUT_SIZE; input_pin_index++) {
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];
    }

    // enable interrupts from output pins
    for (int output_pin_index = 0; output_pin_index < OUTPUT_SIZE; output_pin_index++) {
        LPC_GPIOINT->IO0IntEnF |= 1 << outputPins[output_pin_index];
    }
}

int readKeypad() {
    // set all input pins high
    for (int input_pin_index = 0; input_pin_index < INPUT_SIZE; input_pin_index++) {
        inputPorts[input_pin_index]->FIOSET = 1 << inputPins[input_pin_index];
    }

    int result = 0;
    for (int input_pin_index = 0; input_pin_index < INPUT_SIZE; input_pin_index++) {
        // set one input pin low
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];

        // read state of all four output pins
        for (int output_pin_index = 0; output_pin_index < OUTPUT_SIZE; output_pin_index++) {
            result <<= 1;
            result |= ((outputPort->FIOPIN >> outputPins[output_pin_index]) & 1) ^ 1;
        }

        // set the input put high such that all input pins are high again
        inputPorts[input_pin_index]->FIOSET = 1 << inputPins[input_pin_index];

        for (volatile int i = 200; i > 0; i--);
    }

    clearKeypadPendingRead();

    return result;
}

void clearKeypadPendingRead() {
    // set all input pins low, so that any keypress will cause an interrupt
    for (int input_pin_index = 0; input_pin_index < INPUT_SIZE; input_pin_index++) {
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];
    }

    keypadPendingRead = false;
}

void checkKeypadInterrupt() {
    // get state of falling edge interrupt for all pins in port 0
    int state = LPC_GPIOINT->IO0IntStatF;

    // check if any of the keypad output pins generated an interrupt
    bool ignoreInterrupt = true;
    for (int output_pin_index = 0; output_pin_index < OUTPUT_SIZE; output_pin_index++) {
        if (state & 1 << outputPins[output_pin_index]) {
            ignoreInterrupt = false;
            break;
        }
    }

    if (ignoreInterrupt) {
        // the interrupt was not generated by an output pin; ignore it
        return;
    }

    if (keypadDeadTime == 0) {
        keypadDeadTime = 120;
        keypadPendingRead = true;

        for (volatile int i = 2000; i > 0; i--);
    }

    // clear relevant GPIO interrupt flags
    for (int output_pin_index = 0; output_pin_index < OUTPUT_SIZE; output_pin_index++) {
        LPC_GPIOINT->IO0IntClr = 1 << outputPins[output_pin_index];
    }
}
