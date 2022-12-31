#include "keypad.h"

#include <stdbool.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"


static volatile LPC_GPIO_TypeDef* const INPUT_PORTS[4] = { LPC_GPIO0, LPC_GPIO0, LPC_GPIO1, LPC_GPIO0 };
static const int INPUT_PINS[4] = { 8, 9, 31, 6 };

static volatile LPC_GPIO_TypeDef* const OUTPUT_PORT = LPC_GPIO0;
static const int OUTPUT_PINS[4] = { 17, 18, 15, 16 };

void initializeKeypad() {
    PIN_Configure(0, 6, 0b00, 0b10, true); // 1 COL4
    PIN_Configure(1, 31, 0b00, 0b10, true); // 2 COL3
    PIN_Configure(0, 9, 0b00, 0b10, true); // 3 COL2
    PIN_Configure(0, 8, 0b00, 0b10, true); // 4 COL1

    PIN_Configure(0, 16, 0b00, 0b00, false); // 7 ROW4
    PIN_Configure(0, 15, 0b00, 0b00, false); // 8 ROW3
    PIN_Configure(0, 18, 0b00, 0b00, false); // 9 ROW2
    PIN_Configure(0, 17, 0b00, 0b00, false); // 10 ROW1

    LPC_GPIO0->FIODIR = (1 << 6) | (1 << 8) | (1 << 9);
    LPC_GPIO1->FIODIR = 1 << 31;

    for (int input_pin_index = 0; input_pin_index < sizeof(INPUT_PINS) / sizeof(*INPUT_PINS); input_pin_index++) {
        INPUT_PORTS[input_pin_index]->FIOSET = 1 << INPUT_PINS[input_pin_index];
    }
}

int readKeypad() {
    int result = 0;
    for (int input_pin_index = 0; input_pin_index < sizeof(INPUT_PINS) / sizeof(*INPUT_PINS); input_pin_index++) {
        INPUT_PORTS[input_pin_index]->FIOCLR = 1 << INPUT_PINS[input_pin_index];

        for (int output_pin_index = 0; output_pin_index < sizeof(OUTPUT_PINS) / sizeof(*OUTPUT_PINS); output_pin_index++) {
            result <<= 1;
            result |= ((OUTPUT_PORT->FIOPIN >> OUTPUT_PINS[output_pin_index]) & 1) ^ 1;
        }

        INPUT_PORTS[input_pin_index]->FIOSET = 1 << INPUT_PINS[input_pin_index];

        for (volatile int i = 2000; i > 0; i--);
    }

    return result;
}