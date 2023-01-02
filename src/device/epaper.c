#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "epaper.h"
#include "console.h"


// redirect everything comming from epaper to console
void UART2_IRQHandler() {
	print("received: ");
	while (LPC_UART2->LSR & 1) {
		printChar(LPC_UART2->RBR);
	}
	print("\r\n");
}

void initializeEPaper() {
	LPC_SC->PCONP |= 1 << 24;

	PIN_Configure(0, 10, 0b01, 0, 0);
	PIN_Configure(0, 11, 0b01, 0, 0);

	LPC_UART2->FCR = 1;
	LPC_UART2->LCR = 0b10000011;
	LPC_UART2->DLL = 10;
	LPC_UART2->DLM = 0;
	LPC_UART2->LCR = 0b11;
	LPC_UART2->FDR = (5 << 0) | (14 << 4); // 5/14

	LPC_UART2->IER = 0b1; // enable interrupts
	NVIC_EnableIRQ(UART2_IRQn);
}

void ePaperSendByte(const uint8_t byte) {
	while (!(LPC_UART2->LSR & 1 << 5));
	LPC_UART2->THR = byte;
}

void ePaperSendCommand(const EPaperCommand command, const void* data, const uint16_t dataLength) {
	const uint8_t* dataBytes = data;
	const uint16_t frameLength = dataLength + 9;

	uint8_t parity = 0xa5 ^ (frameLength >> 8) ^ (frameLength & 0xff) ^ command;
	ePaperSendByte(0xa5);
	ePaperSendByte(frameLength >> 8);
	ePaperSendByte(frameLength & 0xff);
	ePaperSendByte(command);
	for (int i = 0; i < dataLength; i++) {
		ePaperSendByte(dataBytes[i]);
		parity ^= dataBytes[i];
	}
	ePaperSendByte(0xcc);
	ePaperSendByte(0x33);
	ePaperSendByte(0xc3);
	ePaperSendByte(0x3c);

	ePaperSendByte(parity);
}
