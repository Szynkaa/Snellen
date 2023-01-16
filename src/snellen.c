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
const uint16_t availableSizes[] = { 10, 20, 30, 40, 50 };
const char charsToOmit[] = { 'I', 'J', 'M', 'Q', 'W', 'X' };

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

            // accept distance
            if (result == CODE_ENTER) {
                break;
            }

            if (inputLength >= sizeof(input) + 1) {
                continue;
            }

            if (result == CODE_BACKSPACE) {
                // remove character
                if (inputLength > 0) {
                    inputLength--;
                    input[inputLength] = '\0';
                }
            }
            else {
                // add character
                input[inputLength] = '0' + result;
                input[inputLength + 1] = '\0';
                inputLength++;
            }

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
    const uint16_t size = availableSizes[letter.sizeIndex];
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
    SnellenTestState testState = { distanceInCm, 0, 0 };
    testState.lastTwoCharacters[0] = '\0';
    testState.lastTwoCharacters[1] = '\0';
    return testState;
}

uint8_t findNearestSizeIndex(const uint16_t size) {
    float diff = 1.0;
    for (int i = 0; i < sizeof(availableSizes) / sizeof(*availableSizes); ++i) {
        const float newDiff = abs(size - availableSizes[i]) / (float)size;
        if (newDiff > diff) return i;
        diff = newDiff;
    }
    return sizeof(availableSizes) / sizeof(*availableSizes) - 1;
}

uint8_t sizeIndexInRange(int8_t index) {
    return min(sizeof(availableSizes) / sizeof(*availableSizes) - 1, max(0, index));
}

char getNewCharacter(const SnellenTestState* testState) {
    char character;
    while (true) {
        character = 'A' + randomNext() % 26;

        bool omittedCharacter = false;
        for (int i = 0; i < sizeof(charsToOmit); i++) {
            if (character == charsToOmit[i]) {
                omittedCharacter = true;
                break;
            }
        }
        if (omittedCharacter)
            continue;

        if (character == testState->lastTwoCharacters[0])
            continue;

        if (character == testState->lastTwoCharacters[1])
            continue;

        break;
    }

    return character;
}

SnellenLetter snellenGetNextLetter(const SnellenTestState* testState) {
    SnellenLetter letter = { getNewCharacter(testState), 0 };

    if (testState->nOfChecksDone == 0) {
        letter.sizeIndex = findNearestSizeIndex(
            testState->distanceInCm * 2 * LETTER_SIZE_TO_DISTANCE_RATIO);
        return letter;
    }

    letter.sizeIndex =
        sizeIndexInRange(testState->shownLettersSizeIndexes[testState->nOfChecksDone - 1]
            + (testState->changeDirection));
    return letter;
}

void snellenUpdateState(SnellenTestState* testState, const SnellenLetter letter, const bool correct) {
    const int8_t indexDiff = testState->nOfChecksDone
        ? testState->shownLettersSizeIndexes[testState->nOfChecksDone - 1] - letter.sizeIndex
        : 0;

    if (testState->nOfChecksDone == 0) {
        testState->changeDirection = correct ? -2 : 2;
    }
    else if (abs(indexDiff) == 2) {
        if (testState->shownLettersCorrectness[testState->nOfChecksDone - 1] != correct)
            testState->changeDirection = testState->changeDirection / -2;
    }
    else {
        testState->changeDirection = correct ? -1 : 1;
    }

    testState->shownLettersSizeIndexes[testState->nOfChecksDone] = letter.sizeIndex;
    testState->shownLettersCorrectness[testState->nOfChecksDone] = correct;
    testState->nOfChecksDone++;

    testState->lastTwoCharacters[1] = testState->lastTwoCharacters[0];
    testState->lastTwoCharacters[0] = letter.character;
}

void snellenCalculateAndShowResult(const SnellenTestState* testState) {
    uint8_t scores[sizeof(availableSizes) / sizeof(*availableSizes)];
    for (int i = 0; i < sizeof(scores) / sizeof(*scores); ++i)
        scores[i] = 0;

    for (int i = 0; i < NUM_OF_TESTS; ++i)
        scores[testState->shownLettersSizeIndexes[i]] += testState->shownLettersCorrectness[i];

    int i = 0;
    for (;i < sizeof(scores) / sizeof(*scores); ++i)
        if (scores[i]) break;

    // result size is average of two best sizes with number of correct guesses as weight
    unsigned scoreSum = 0;
    uint8_t scoreCounter = 0;
    for (int k = 0; k < 2 && i < sizeof(scores); ++k, ++i) {
        scoreCounter += scores[i];
        scoreSum += scores[i] * availableSizes[i];
    }

    char textData[16] = "\x00\x20\x00\x20try again";
    int textLength = 9;
    if (scoreCounter != 0) {
        // convert to x in 6/x result notation
        const float result = ((float)scoreSum / scoreCounter) / (testState->distanceInCm * LETTER_SIZE_TO_DISTANCE_RATIO) * 6;
        textLength = sprintf(textData + 4, "6/%d", (int)round(result));
    }

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperSendCommand(EPAPER_DISPLAY_TEXT, textData, textLength + 5);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);
}
