#include <stdio.h>

#include "snellen.h"
#include "device/epaper.h"

#define X_SIZE 800
#define Y_SIZE 600

int displaySnellenLetter(const char letter, const unsigned int size) {
    if (letter < 'A' || letter > 'Z') {
        return 1;
    }

    const int xpos = (X_SIZE - size) / 2;
    const int ypos = (Y_SIZE - size) / 2;

    char imageFilename[32];
    const int imageFilenameLength = sprintf(imageFilename, "%c_%d.bmp", letter, size);

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperDisplayImage(xpos, ypos, imageFilename);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    return 0;
}
