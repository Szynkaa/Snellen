#ifndef _SNELLEN_
#define _SNELLEN_

#include <stdbool.h>

#define SIZES_COUNT 8

// If currentSizeLowerBoundIndex and currentSizeUpperBoundIndex differ by more than 1,
// the state is in the binary search phase, in which an initial size index is searched
// for to start testing from. Otherwise, the state is in the concentrated phase, where
// for a given size letters are shown until LETTERS_SHOWN_PER_SIZE letters have been
// reached.
typedef struct {
    unsigned int distanceInCm;
    unsigned char currentSizeLowerBoundIndex;
    unsigned char currentSizeUpperBoundIndex;
    unsigned char shownLettersBySize[SIZES_COUNT];
    unsigned char correctLettersBySize[SIZES_COUNT];
} SnellenTestState;

typedef struct {
    char character;
    short sizeIndex;
} SnellenShownLetter;

SnellenTestState snellenCreateTestState(int distanceInCm);
void snellenDisplayLetter(const SnellenTestState* testState, SnellenShownLetter letter);
SnellenShownLetter snellenGetNextLetter(const SnellenTestState* testState);
void snellenUpdateState(SnellenTestState* testState, const SnellenShownLetter letter, const bool correct);

#endif // _SNELLEN_
