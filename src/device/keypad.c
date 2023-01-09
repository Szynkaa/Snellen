#include "keypad.h"
#include "console.h"

#include <stddef.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"


volatile bool readLengthFromKeypad = false;

volatile int keypadDeadTime = 0;

static const int inputSize = 4;
static volatile LPC_GPIO_TypeDef* const inputPorts[inputSize] = { LPC_GPIO0, LPC_GPIO0, LPC_GPIO1, LPC_GPIO0 };
static const uint8_t inputPins[inputSize] = { 8, 9, 31, 6 };

static const int outputSize = 4;
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

    for (int input_pin_index = 0; input_pin_index < inputSize; input_pin_index++) {
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];
    }

    // enable interrupts from output pins
    for (int output_pin_index = 0; output_pin_index < outputSize; output_pin_index++) {
        LPC_GPIOINT->IO0IntEnF |= 1 << outputPins[output_pin_index];
    }
}

int readKeypad() {
    print("keypad read\r\n");
    
    for (volatile int i = 2000; i > 0; i--);

    // set all input pins high
    for (int input_pin_index = 0; input_pin_index < inputSize; input_pin_index++) {
        inputPorts[input_pin_index]->FIOSET = 1 << inputPins[input_pin_index];
    }
    
    for (volatile int i = 2000; i > 0; i--);
    
    int result = 0;
    for (int input_pin_index = 0; input_pin_index < inputSize; input_pin_index++) {
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];

        for (int output_pin_index = 0; output_pin_index < outputSize; output_pin_index++) {
            result <<= 1;
            result |= ((outputPort->FIOPIN >> outputPins[output_pin_index]) & 1) ^ 1;
        }

        inputPorts[input_pin_index]->FIOSET = 1 << inputPins[input_pin_index];

        for (volatile int i = 2000; i > 0; i--);
    }
    
    // set all input pins to open drain
    for (int input_pin_index = 0; input_pin_index < inputSize; input_pin_index++) {
        inputPorts[input_pin_index]->FIOCLR = 1 << inputPins[input_pin_index];
    }
    
    readLengthFromKeypad = false;
    
    return result;
}

void checkKeypadInterrupt() {    
    int state = LPC_GPIOINT->IO0IntStatF;
    
    bool interruptPresent = false;
    for (int output_pin_index = 0; output_pin_index < outputSize; output_pin_index++) {
        if (state & 1 << outputPins[output_pin_index]) {
            interruptPresent = true;
        }
    }
    
    if (keypadDeadTime > 0) {
        LPC_GPIOINT->IO0IntClr = (1 << outputPins[0]) | (1 << outputPins[1]) | (1 << outputPins[2]) | (1 << outputPins[3]);
        
        return;
    }
    
    if (!interruptPresent) {
        LPC_GPIOINT->IO0IntClr = (1 << outputPins[0]) | (1 << outputPins[1]) | (1 << outputPins[2]) | (1 << outputPins[3]);
        
        return;
    }
    
    keypadDeadTime = 120;
    readLengthFromKeypad = interruptPresent;
    
    for (volatile int i = 2000; i > 0; i--);
    
    LPC_GPIOINT->IO0IntClr = (1 << outputPins[0]) | (1 << outputPins[1]) | (1 << outputPins[2]) | (1 << outputPins[3]);
}
