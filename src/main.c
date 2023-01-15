#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "board/systick.h"
#include "board/keys.h"
#include "device/console.h"
#include "device/epaper.h"
#include "device/keypad.h"
#include "random.h"
#include "snellen.h"
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
    randomSetSeed(4079839681u);

    initializeKeys();
    key0Callback = key0Handler;
    key1Callback = key1Handler;

    NVIC_EnableIRQ(EINT3_IRQn);

    // Main logic
    print("**program initialized**\r\n");

    const unsigned char colorData[] = { 0x00, 0x03 };
    ePaperSendCommand(EPAPER_SET_COLORS, colorData, sizeof(colorData));

    const unsigned char fontSize[] = { 0x03 };
    ePaperSendCommand(EPAPER_SET_FONT_SIZE, fontSize, sizeof(fontSize));

    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    const uint8_t storageAreaId = 1;
    ePaperSendCommand(EPAPER_SET_STORAGE_AREA, &storageAreaId, 1);

    char input[128];
    int inputLength = 0;
    while (true) {
        if (keypadPendingRead) {
            const int result = keypadCodeToDigit(readKeypad());

            if (result == CODE_IGNORE) {
                // ignore
                continue;
            }
            else if (result == CODE_BACKSPACE) {
                // remove character
                if (inputLength > 0) {
                    inputLength--;
                    input[inputLength] = '\0';
                }
            }
            else if (result == CODE_ENTER) {
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
        randomNext(); // cycle prng constantly
    }

    char printBuffer[64];
    SnellenTestState testState = snellenCreateTestState(600); // TODO: use input from earlier

    while (true) {
        SnellenShownLetter shownLetter = snellenGetNextLetter(&testState);
        if (shownLetter.character == '\0') {
            break;
        }

        sprintf(printBuffer, "Showing character %c with size index %d\r\n", shownLetter.character, shownLetter.sizeIndex);
        print(printBuffer);

        snellenDisplayLetter(&testState, shownLetter);

        bool isCorrect;
        while (true) {
            while (!keypadPendingRead) {
                randomNext(); // cycle prng constantly
            }
            int pressedKey = keypadCodeToDigit(readKeypad());
            if (pressedKey == -1) { // C
                isCorrect = true;
                break;
            }
            else if (pressedKey == -2) { // D
                isCorrect = false;
                break;
            }
        }

        sprintf(printBuffer, "Letter marked as %s\r\n", isCorrect ? "correct" : "wrong");
        print(printBuffer);

        snellenUpdateState(&testState, shownLetter, isCorrect);
    }
}
