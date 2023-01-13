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

    char image[32];
    const int imageLength = sprintf(image, "%02x%02x%c_%d.bmp", xpos, ypos, letter, size);

    ePaperSendCommand(EPAPER_CLEAR_SCREEN, NULL, 0);
    ePaperSendCommand(EPAPER_DISPLAY_IMAGE, image, imageLength + 1);
    ePaperSendCommand(EPAPER_REFRESH, NULL, 0);

    return 0;
}
