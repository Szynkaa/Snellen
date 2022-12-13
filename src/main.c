#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "console.h"
#include "epaper.h"
#include "util.h"


volatile int eint1DeadTime = 0;

void EINT1_IRQHandler() {
	LPC_SC->EXTINT = 1 << 1;
	
	if(eint1DeadTime > 0) {
		return;
	}
	
	printChar('.');
	eint1DeadTime = 200;
}

void SysTick_Handler() {
	if (eint1DeadTime > 0) {
		--eint1DeadTime;
	}
}

void UART2_IRQHandler()
{
	print("received: ");
	while (LPC_UART2->LSR & 1)
	{
		printChar(LPC_UART2->RBR);
	}
	print("\r\n");
}

int readKeypad() {
	static volatile LPC_GPIO_TypeDef * const INPUT_PORTS[4] = {LPC_GPIO0, LPC_GPIO0, LPC_GPIO1, LPC_GPIO0};
	static const int INPUT_PINS[4] = {8, 9, 31, 6};
	
	static volatile LPC_GPIO_TypeDef * const OUTPUT_PORT = LPC_GPIO0;
	static const int OUTPUT_PINS[4] = {17, 18, 15, 16};
	
	
	for (int input_pin_index = 0; input_pin_index < sizeof(INPUT_PINS) / sizeof(*INPUT_PINS); input_pin_index++) {
		INPUT_PORTS[input_pin_index]->FIOSET = 1 << INPUT_PINS[input_pin_index];
	}
	
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

int main()
{
	initializeSysTick(1, 1000);
	initializeConsole();
	initializeEPaper();
	
	// Key 1
	PIN_Configure(2, 11, 0b01, 0, 0);
	
	LPC_SC->EXTMODE = (LPC_SC->EXTMODE & 0b1111) | 1 << 1;
	NVIC_EnableIRQ(EINT1_IRQn);
	
	// Main logic
	print("Ping\r\n");
	
//	ePaperSendCommand(EPAPER_HANDSHAKE, NULL, 0);
//	ePaperSendCommand(EPAPER_GET_BAUD_RATE, NULL, 0);

//	const unsigned char colorData[] = {0x00, 0x03};
//	ePaperSendCommand(EPAPER_SET_COLORS, colorData, sizeof(colorData));
//	
//	const unsigned char fontSize[] = {0x03};
//	ePaperSendCommand(EPAPER_SET_FONT_SIZE, fontSize, sizeof(fontSize));
//	
//	ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
//	
//	for (volatile int i = 100000000; i > 0; i--);
//	
//	const char text[] = "\x00\x20\x00\x20\xC4\xE3\xBA\xC3\x57\x6F\x72\x6C\x64";
//	ePaperSendCommand(EPAPER_DISPLAY_TEXT, text, sizeof(text));
//	
//	const uint16_t linePoints[] = {swapEndianness(0), swapEndianness(0), swapEndianness(256), swapEndianness(256)};
//	ePaperSendCommand(EPAPER_DRAW_LINE, linePoints, sizeof(linePoints));
//	
//	const uint8_t storageAreaId = 1;
//	ePaperSendCommand(EPAPER_SET_STORAGE_AREA, &storageAreaId, 1);
//	
//	const uint8_t imageData[] = "\0\0\0\0B_600.bmp";
//	ePaperSendCommand(EPAPER_DISPLAY_IMAGE, imageData, sizeof(imageData));
//	
//	ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

	// Keypad
	
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
	
	
	while (true) {
		int result = readKeypad();
		
		char buffer[32];
		sprintf(buffer, "%04x\r\n", result);
		print(buffer);
	}
}
