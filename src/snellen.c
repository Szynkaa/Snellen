#include <stdio.h>

#include "device/epaper.h"
#include "random.h"
#include "snellen.h"

#define X_SIZE 800
#define Y_SIZE 600

// multiply distance in centimeters by this value to get letter size in pixels for a 6/6 test
#define LETTER_SIZE_TO_DISTANCE_RATIO 0.13222193633f

#define LETTERS_SHOWN_PER_SIZE 5
#define CORRECT_LETTERS_PER_SIZE_TO_PASS 3

const float sizeMultipliers[SIZES_COUNT] = { 9, 6, 4, 3, 2, 1, 0.75, 0.5 };

int snellenDisplayLetter(const SnellenTestState* testState, SnellenShownLetter letter) {
    if (letter.character < 'A' || letter.character > 'Z') {
        return 1;
    }

    // TODO: adjust letter size to one for which there are images
    const int letterSize = testState->distanceInCm
        * sizeMultipliers[letter.sizeIndex]
        * LETTER_SIZE_TO_DISTANCE_RATIO
        + 0.5; // + 0.5 to round to nearest
    const int xpos = (X_SIZE - letterSize) / 2;
    const int ypos = (Y_SIZE - letterSize) / 2;

    char imageFilename[32];
    const int imageFilenameLength = sprintf(imageFilename, "%c_%d.bmp", letter.character, letterSize);

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperDisplayImage(xpos, ypos, imageFilename);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    return 0;
}

SnellenTestState snellenCreateTestState(int distanceInCm) {
    SnellenTestState testState = {};
    testState.distanceInCm = distanceInCm;
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
        SnellenShownLetter shownLetter = { 'A' + (randomNext() % 26), sizeIndex };
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
