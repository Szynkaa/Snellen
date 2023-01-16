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

void EINT3_IRQHandler() {
    LPC_SC->EXTINT = 1 << 3; // clear external interrupt flag
    checkKeypadInterrupt();
}

volatile int letterGuessedCorrectly;

void key0Handler() {
    letterGuessedCorrectly = -1;
}

void key1Handler() {
    letterGuessedCorrectly = 1;
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

    const uint16_t distance = getDistanceFromUser();
    SnellenTestState testState = snellenCreateTestState(distance);

    char printBuffer[64];
    while (true) {
        if (testState.nOfChecksDone == NUM_OF_TESTS) {
            break;
        }

        SnellenLetter letter = snellenGetNextLetter(&testState);
        snellenDisplayLetter(letter);
        waitSysTick(2000);

        sprintf(printBuffer, "Showing character %c with sizeIndex %d\r\n", letter.character, letter.sizeIndex);
        print(printBuffer);

        letterGuessedCorrectly = 0;
        while (letterGuessedCorrectly == 0);
        const bool isCorrect = letterGuessedCorrectly > 0;

        sprintf(printBuffer, "Letter marked as %s\r\n", isCorrect ? "correct" : "wrong");
        print(printBuffer);

        snellenUpdateState(&testState, letter, isCorrect);
    }

    snellenCalculateAndShowResult(&testState);

    while (true);
}
