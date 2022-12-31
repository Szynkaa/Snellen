#include "util.h"


inline uint16_t swapEndianness(uint16_t x) {
	return (x << 8) | (x >> 8);
}
