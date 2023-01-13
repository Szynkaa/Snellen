#ifndef _EPAPER_
#define _EPAPER_

#include <stdint.h>

typedef enum {
	EPAPER_HANDSHAKE = 0x00,
	EPAPER_SET_BAUD_RATE = 0x01,
	EPAPER_GET_BAUD_RATE = 0x02,
	EPAPER_SET_STORAGE_AREA = 0x07,
	EPAPER_REFRESH = 0x0A,
	EPAPER_SET_COLORS = 0x10,
	EPAPER_SET_FONT_SIZE = 0x1E,
	EPAPER_SET_CHINESE_FONT_SIZE = 0x1F,
	EPAPER_DRAW_LINE = 0x22,
	EPAPER_FILL_RECTANGLE = 0x24,
	EPAPER_DRAW_RECTANGLE = 0x25,
	EPAPER_DRAW_CIRCLE = 0x26,
	EPAPER_FILL_CIRCLE = 0x27,
	EPAPER_DRAW_TRIANGLE = 0x28,
	EPAPER_FILL_TRIANGLE = 0x29,
	EPAPER_CLEAR_SCREEN = 0x2E,
	EPAPER_DISPLAY_TEXT = 0x30,
	EPAPER_DISPLAY_IMAGE = 0x70,
} EPaperCommand;

void initializeEPaper();
void ePaperSendCommand(const EPaperCommand command, const void* data, const uint16_t dataLength);
void ePaperDisplayImage(unsigned short x, unsigned short y, const char* filename);

#endif // _EPAPER_
