#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "device/epaper.h"
#include "device/keypad.h"
#include "device/console.h"
#include "util.h"
#include "random.h"
#include "snellen.h"

#define X_SIZE 800
#define Y_SIZE 600

// multiply distance in centimeters by this value to get letter size in pixels for a 6/6 test
#define LETTER_SIZE_TO_DISTANCE_RATIO 0.13222193633f

// TODO fill this according to generated letters in ascending order
const uint16_t avaliableSizes[] = { 10, 20, 30, 40, 50 };

uint16_t getDistanceFromUser() {
    char input[4];
    int inputLength = 0;
    while (true) {
        if (keypadPendingRead) {
            const int result = keypadCodeToDigit(readKeypad());

            // ignore wrong code
            if (result == CODE_IGNORE) {
                continue;
            }

            // remove character
            if (result == CODE_BACKSPACE) {
                if (inputLength > 0) {
                    inputLength--;
                    input[inputLength] = '\0';
                }
                continue;
            }

            // accept distance
            if (result == CODE_ENTER) {
                break;
            }

            if (inputLength >= sizeof(input) + 1) {
                continue;
            }

            // add character
            input[inputLength] = '0' + result;
            input[inputLength + 1] = '\0';
            inputLength++;

            print(input);
            print("\r\n");

            // TODO show as meters?
            char textData[4 + sizeof(input)] = "\x00\x20\x00\x20";
            strcpy(textData + 4, input);
            ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
            ePaperSendCommand(EPAPER_DISPLAY_TEXT, textData, strlen(input) + 5);
            ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

            char buffer[32];
            sprintf(buffer, "strlen: %d\r\n", strlen(input) + 5);
            print(buffer);
        }
        randomNext(); // cycle prng constantly
    }
    return atoi(input);
}

void snellenDisplayLetter(SnellenLetter letter) {
    const uint16_t size = avaliableSizes[letter.sizeIndex];
    const uint16_t xpos = swapEndianness((X_SIZE - size) / 2);
    const uint16_t ypos = swapEndianness((Y_SIZE - size) / 2);

    char data[16];
    memcpy(data, &xpos, sizeof(xpos));
    memcpy(data + 2, &ypos, sizeof(ypos));
    const int filenameLength = sprintf(data + 4, "%c_%d.bmp", letter.character, size);

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperSendCommand(EPAPER_DISPLAY_IMAGE, data, filenameLength + 5);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);
}

SnellenTestState snellenCreateTestState(int distanceInCm) {
    SnellenTestState testState = { distanceInCm, 0 };
    testState.lastTwoCharacters[0] = '\0';
    testState.lastTwoCharacters[1] = '\0';
    return testState;
}

uint8_t findNearestSizeIndex(const uint16_t size) {
    float diff = 1.0;
    for (int i = 0; i < sizeof(avaliableSizes); ++i) {
        const float newDiff = abs(size - avaliableSizes[i]) / (float)size;
        if (newDiff > diff) return i;
        diff = newDiff;
    }
    return sizeof(avaliableSizes) - 1;
}

uint8_t sizeIndexInRange(uint8_t index) {
    return min(sizeof(avaliableSizes), max(0, index));
}

// basic size changing
SnellenLetter snellenGetNextLetter(const SnellenTestState* testState) {
    SnellenLetter letter = { 'A' + (randomNext() % 26), 0 };
    while (letter.character == testState->lastTwoCharacters[0]
        || letter.character == testState->lastTwoCharacters[1]) {
        letter.character = 'A' + randomNext() % 26;
    }

    if (testState->nOfChecksDone == 0) {
        letter.sizeIndex = findNearestSizeIndex(
            testState->distanceInCm * LETTER_SIZE_TO_DISTANCE_RATIO);
        return letter;
    }

    if (testState->nOfChecksDone == 1) {
        letter.sizeIndex =
            sizeIndexInRange(testState->shownLettersSizeIndexes[0]
                + (testState->shownLettersCorrectness[0] ? -2 : 2));
        return letter;
    }

    letter.sizeIndex =
        sizeIndexInRange(testState->shownLettersSizeIndexes[0]
            + (testState->shownLettersCorrectness[0] ? -1 : 1));
    return letter;
}

void snellenUpdateState(SnellenTestState* testState, const SnellenLetter letter, const bool correct) {
    testState->shownLettersSizeIndexes[testState->nOfChecksDone] = letter.sizeIndex;
    testState->shownLettersCorrectness[testState->nOfChecksDone] = correct;
    testState->nOfChecksDone++;

    testState->lastTwoCharacters[1] = testState->lastTwoCharacters[0];
    testState->lastTwoCharacters[0] = letter.character;
}
