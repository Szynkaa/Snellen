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

	const unsigned char colorData[] = {0x00, 0x03};
	ePaperSendCommand(EPAPER_SET_COLORS, colorData, sizeof(colorData));
	
//	const unsigned char fontSize[] = {0x03};
//	ePaperSendCommand(EPAPER_SET_FONT_SIZE, fontSize, sizeof(fontSize));
	
	ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
	
//	for (volatile int i = 100000000; i > 0; i--);
	
//	const char text[] = "\x00\x20\x00\x20\xC4\xE3\xBA\xC3\x57\x6F\x72\x6C\x64";
//	ePaperSendCommand(EPAPER_DISPLAY_TEXT, text, sizeof(text));
//	
//	const uint16_t linePoints[] = {swapEndianness(0), swapEndianness(0), swapEndianness(256), swapEndianness(256)};
//	ePaperSendCommand(EPAPER_DRAW_LINE, linePoints, sizeof(linePoints));
	
	const uint8_t storageAreaId = 1;
	ePaperSendCommand(EPAPER_SET_STORAGE_AREA, &storageAreaId, 1);
	
	const uint8_t imageData[] = "\0\0\0\0B_600.bmp";
	ePaperSendCommand(EPAPER_DISPLAY_IMAGE, imageData, sizeof(imageData));
	
	ePaperSendCommand(EPAPER_REFRESH, NULL, 0);
	
	while (true);
}
