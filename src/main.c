#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "LPC17xx.h"
#include "board/systick.h"
#include "board/keys.h"
#include "device/console.h"
#include "device/epaper.h"
#include "device/keypad.h"
#include "util.h"

volatile int key0DeadTime = 0;
volatile int key1DeadTime = 0;

void EINT3_IRQHandler() {
    LPC_SC->EXTINT = 1 << 3; // clear external interrupt flag
    checkKeypadInterrupt();
}

void key0Handler() {
    printChar('0');
}

void key1Handler() {
    printChar('1');
}

int main() {
    initializeSysTick(1, 1000);
    initializeConsole();
    initializeEPaper();
    initializeKeypad();

    initializeKeys();
    key0Callback = key0Handler;
    key1Callback = key1Handler;

    // Main logic
    print("Ping\r\n");

    const unsigned char colorData[] = { 0x00, 0x03 };
    ePaperSendCommand(EPAPER_SET_COLORS, colorData, sizeof(colorData));

    const unsigned char fontSize[] = { 0x03 };
    ePaperSendCommand(EPAPER_SET_FONT_SIZE, fontSize, sizeof(fontSize));

    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    const uint8_t storageAreaId = 1;
    ePaperSendCommand(EPAPER_SET_STORAGE_AREA, &storageAreaId, 1);

    NVIC_EnableIRQ(EINT3_IRQn);

    char input[128];
    int inputLength = 0;
    while (true) {

        if (keypadPendingRead) {
            int result = keypadCodeToDigit(readKeypad());

            if (result == -3) {
                // ignore
                continue;
            }
            else if (result == -1) {
                // remove character
                if (inputLength > 0) {
                    inputLength--;
                    input[inputLength] = '\0';
                }
            }
            else if (result == -2) {
                // accept
                break;
            }
            else {
                // add character
                input[inputLength] = '0' + result;
                input[inputLength + 1] = '\0';
                inputLength++;
            }
            print(input);
            print("\r\n");

            ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
            char textData[132] = "\x00\x20\x00\x20";
            strcpy(textData + 4, input);
            ePaperSendCommand(EPAPER_DISPLAY_TEXT, textData, strlen(input) + 5);

            char buffer[32];
            sprintf(buffer, "strlen: %d\r\n", strlen(input) + 5);
            print(buffer);

            ePaperSendCommand(EPAPER_REFRESH, NULL, 0);
        }
    }

    const uint8_t imageData[] = "\0\0\0\0A_400.bmp";
    ePaperSendCommand(EPAPER_DISPLAY_IMAGE, imageData, sizeof(imageData));

    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    while (true) {

    }
}
