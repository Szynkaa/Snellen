#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "epaper.h"
#include "console.h"

#define COMMAND_HISTORY_SIZE 128


EPaperCommand commandHistory[COMMAND_HISTORY_SIZE];
int commandHistoryStart = 0;
int commandHistoryEnd = 0;

char errorCodeBuffer[4] = {};
int errorCodeLength = 0;
	
void printError();

void commandHistoryEnqueue(EPaperCommand command) {
	commandHistoryEnd = (commandHistoryEnd + 1) % COMMAND_HISTORY_SIZE;
	commandHistory[commandHistoryEnd] = command;
}

EPaperCommand commandHistoryDequeue() {
	EPaperCommand command = commandHistory[commandHistoryStart];
	commandHistoryStart = (commandHistoryStart + 1) % COMMAND_HISTORY_SIZE;
	return command;
}
	
// redirect everything coming in from epaper to console
void UART2_IRQHandler() {
	char received = LPC_UART2->RBR;
	
	char buffer[32];
	switch (received) {
		case 'O':
			LPC_TIM0->TCR = 0;
			if (errorCodeLength > 0) {
				printError();
			}
		
			sprintf(buffer, "%02x: OK\r\n", commandHistoryDequeue());
			print(buffer);
			break;
			
		case 'E':
			LPC_TIM0->TCR = 0;
			if (errorCodeLength > 0) {
				printError();
			}
			break;
		
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			errorCodeBuffer[errorCodeLength] = received;
			errorCodeLength++;
		
			// reset and start timer 0
			LPC_TIM0->TCR = 0b11; 
			LPC_TIM0->TCR = 0b1;
		
			break;
		
		default:
			break;
	}
}

void TIMER0_IRQHandler() {
	LPC_TIM0->IR = 1;
	
	printError();
}

void printError() {
	char buffer[32];
	sprintf(buffer, "%02x: ERROR %s\r\n", commandHistoryDequeue(), errorCodeBuffer);
	print(buffer);

	errorCodeLength = 0;
	memset(errorCodeBuffer, 0, sizeof(errorCodeBuffer));
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
	commandHistoryEnqueue(command);
	
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
