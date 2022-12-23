#include <string.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "console.h"


void initializeConsole() {
	PIN_Configure(0, 2, 0b01, 0, 0);
	PIN_Configure(0, 3, 0b01, 0, 0);

	LPC_UART0->FCR = 1;
	LPC_UART0->LCR = 0b10000011;
	LPC_UART0->DLL = 10;
	LPC_UART0->DLM = 0;
	LPC_UART0->LCR = 0b11;
	LPC_UART0->FDR = (5 << 0) | (14 << 4); // 5/14
}

void print(const char* string) {
	for (const char* c = string; *c != '\0'; c++) {
		printChar(*c);
	}
}

void printN(const char* string, int length) {
	for (int i = 0; i < length; i++) {
		printChar(string[i]);
	}
}

void printChar(char character) {
	while (!(LPC_UART0->LSR & 1 << 5));
	LPC_UART0->THR = character;
}
