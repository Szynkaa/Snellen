#ifndef _SNELLEN_
#define _SNELLEN_

#include <stdbool.h>

#define NUM_OF_TESTS 10

typedef struct SnellenTestState {
    uint16_t distanceInCm;
    uint8_t nOfChecksDone;
    int8_t changeDirection;
    char lastTwoCharacters[2];
    uint8_t shownLettersSizeIndexes[NUM_OF_TESTS];
    bool shownLettersCorrectness[NUM_OF_TESTS];
} SnellenTestState;

typedef struct SnellenLetter {
    char character;
    uint8_t sizeIndex;
} SnellenLetter;

uint16_t getDistanceFromUser();
void displayDistanceScreen(uint16_t distance);
SnellenTestState snellenCreateTestState(int distanceInCm);
void snellenDisplayLetter(SnellenLetter letter);
SnellenLetter snellenGetNextLetter(const SnellenTestState* testState);
void snellenUpdateState(SnellenTestState* testState, const SnellenLetter letter, const bool correct);
void snellenCalculateAndShowResult(const SnellenTestState* testState);

#endif // _SNELLEN_
