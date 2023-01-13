#include <stdbool.h>
#include <stdio.h>

#include "snellen.h"
#include "device/epaper.h"

#define X_SIZE 800
#define Y_SIZE 600

#define LETTERS_SHOWN_PER_SIZE 5
#define CORRECT_LETTERS_PER_SIZE_TO_PASS 3

const unsigned short sizes[SIZES_COUNT] = { 600, 500, 400 };

int snellenDisplayLetter(SnellenShownLetter letter) {
    if (letter.character < 'A' || letter.character > 'Z') {
        return 1;
    }

    const int xpos = (X_SIZE - sizes[letter.sizeIndex]) / 2;
    const int ypos = (Y_SIZE - sizes[letter.sizeIndex]) / 2;

    char imageFilename[32];
    const int imageFilenameLength = sprintf(imageFilename, "%c_%d.bmp", letter.character, sizes[letter.sizeIndex]);

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperDisplayImage(xpos, ypos, imageFilename);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    return 0;
}

SnellenTestState snellenCreateTestState() {
    SnellenTestState testState = {};
    testState.currentSizeLowerBoundIndex = 0;
    testState.currentSizeUpperBoundIndex = SIZES_COUNT;
    return testState;
}

SnellenShownLetter snellenGetNextLetter(const SnellenTestState* testState) {
    int sizeIndex = -1;

    if (testState->currentSizeUpperBoundIndex - testState->currentSizeLowerBoundIndex >= 2) {
        // binary search phase
        sizeIndex = (testState->currentSizeLowerBoundIndex + testState->currentSizeUpperBoundIndex) / 2;
    }
    else {
        // concentrated phase
        if (testState->shownLettersBySize[testState->currentSizeLowerBoundIndex] < LETTERS_SHOWN_PER_SIZE) {
            // continue with the current size
            sizeIndex = testState->currentSizeLowerBoundIndex;
        }
        else {
            // enough letters shown for current size; change size index
            sizeIndex = testState->correctLettersBySize[testState->currentSizeLowerBoundIndex] >= CORRECT_LETTERS_PER_SIZE_TO_PASS
                ? testState->currentSizeLowerBoundIndex + 1 : testState->currentSizeLowerBoundIndex - 1; // previous size if incorrect, next size if correct

            if (sizeIndex < 0 || sizeIndex >= SIZES_COUNT) {
                // size doesn't exist; no more letters need to be shown in the test
                sizeIndex = -1;
            }
            else if (testState->shownLettersBySize[sizeIndex] >= LETTERS_SHOWN_PER_SIZE) {
                // new size has already been completed; no more letters need to be shown in the test
                sizeIndex = -1;
            }
        }
    }

    if (sizeIndex < 0) {
        SnellenShownLetter nullShownLetter = { '\0', 0 };
        return nullShownLetter;
    }
    else {
        SnellenShownLetter shownLetter = { 'A' + testState->shownLettersBySize[sizeIndex] , sizeIndex };
        return shownLetter;
    }
}

void snellenUpdateState(SnellenTestState* testState, const SnellenShownLetter letter, const bool correct) {
    if (testState->currentSizeUpperBoundIndex - testState->currentSizeLowerBoundIndex >= 2) {
        // binary search phase
        if (correct) {
            testState->currentSizeLowerBoundIndex = letter.sizeIndex;
        }
        else {
            testState->currentSizeUpperBoundIndex = letter.sizeIndex;
        }
    }
    else {
        // concentrated phase
        testState->currentSizeLowerBoundIndex = letter.sizeIndex;
        testState->currentSizeUpperBoundIndex = letter.sizeIndex + 1;
    }

    testState->shownLettersBySize[letter.sizeIndex]++;
    if (correct) {
        testState->correctLettersBySize[letter.sizeIndex]++;
    }
}
