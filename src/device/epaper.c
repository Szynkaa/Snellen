#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "PIN_LPC17xx.h"
#include "epaper.h"
#include "console.h"

// "private" functions declarations
void printErrorAndClearErrorCodeBuffer();
void commandHistoryEnqueue(const EPaperCommand command);
EPaperCommand commandHistoryDequeue();
void sendNewEPaperRefreshIfPending();
bool updateRefreshStatus();

#define COMMAND_HISTORY_SIZE 128

volatile EPaperCommand commandHistory[COMMAND_HISTORY_SIZE];
volatile int commandHistoryStart = 0;
volatile int commandHistoryEnd = 0;

volatile char errorCodeBuffer[4] = {};
volatile int errorCodeLength = 0;

volatile enum EPaperRefreshStatus {
    NO_REFRESH_IN_QUEUE = 0,
    REFRESH_IN_QUEUE = 1,
    REFRESH_PENDING = 2
} ePaperRefreshStatus = NO_REFRESH_IN_QUEUE;

void commandHistoryEnqueue(const EPaperCommand command) {
    commandHistoryEnd = (commandHistoryEnd + 1) % COMMAND_HISTORY_SIZE;
    commandHistory[commandHistoryEnd] = command;
}

EPaperCommand commandHistoryDequeue() {
    const EPaperCommand command = commandHistory[commandHistoryStart];
    if (command == EPAPER_REFRESH) {
        sendNewEPaperRefreshIfPending();
    }
    commandHistoryStart = (commandHistoryStart + 1) % COMMAND_HISTORY_SIZE;
    return command;
}

void sendNewEPaperRefreshIfPending() {
    if (ePaperRefreshStatus == REFRESH_IN_QUEUE) {
        ePaperRefreshStatus = NO_REFRESH_IN_QUEUE;
    }
    else if (ePaperRefreshStatus == REFRESH_PENDING) {
        ePaperRefreshStatus = NO_REFRESH_IN_QUEUE;
        ePaperSendCommand(EPAPER_REFRESH, NULL, 0);
    }
}

bool updateRefreshStatus() {
    return ePaperRefreshStatus == REFRESH_PENDING || ePaperRefreshStatus++;
}

// redirect epaper feedback to console with respective EPaperCommand code
void UART2_IRQHandler() {
    while (LPC_UART2->LSR & 1) {
        const char received = LPC_UART2->RBR;

        if (received == 'k') {
            char buffer[32];
            sprintf(buffer, "%02x: OK\r\n", commandHistoryDequeue());
            print(buffer);
            continue;
        }

        if (received == 'O' || received == 'E') {
            LPC_TIM0->TCR = 0b10;
            if (errorCodeLength > 0) {
                printErrorAndClearErrorCodeBuffer();
            }
            continue;
        }

        if (received >= '0' && received <= '9') {
            errorCodeBuffer[errorCodeLength] = received;
            errorCodeLength++;

            // reset and start timer 0
            LPC_TIM0->TCR = 0b11;
            LPC_TIM0->TCR = 0b01;

            continue;
        }
    }
}

void TIMER0_IRQHandler() {
    LPC_TIM0->IR = 1;
    printErrorAndClearErrorCodeBuffer();
}

void printErrorAndClearErrorCodeBuffer() {
    char buffer[32];
    sprintf(buffer, "%02x: ERROR %s\r\n", commandHistoryDequeue(), errorCodeBuffer);
    print(buffer);

    errorCodeLength = 0;
    for (int i = 0; i < sizeof(errorCodeBuffer); ++i) errorCodeBuffer[i] = 0;
}

void initializeEPaper() {
    // setup UART2 as communication with epaper
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

    // setup Timer 0 as timeout for epaper feedback
    LPC_TIM0->PR = SystemCoreClock / 4 / 1000000; // 1 microsecond
    LPC_TIM0->MR0 = 100; // 100us
    LPC_TIM0->MCR = 0b101;

    NVIC_EnableIRQ(TIMER0_IRQn);
}

void ePaperSendByte(const uint8_t byte) {
    while (!(LPC_UART2->LSR & 1 << 5));
    LPC_UART2->THR = byte;
}

void ePaperSendCommand(const EPaperCommand command, const void* data, const uint16_t dataLength) {
    if (command == EPAPER_REFRESH && updateRefreshStatus()) {
        return;
    }

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
