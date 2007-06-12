#include "test.h"

int main() {
	uint8_t b00, b01, b02, b03, b04, b05, b06, b07, b08, b09, b10, b11, b12, b13, b14, b15;

	LOAD_VECTOR_UBYTE(13, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
	LOAD_VECTOR_UBYTE(14, 4, 2, 8, 6, 12, 10, 16, 14, 20, 18, 24, 22, 28, 26, 32, 30);

	asm("vmaxub 15, 13, 14");

	STORE_VECTOR_UBYTE(15, b00, b01, b02, b03, b04, b05, b06, b07, b08, b09, b10, b11, b12, b13, b14, b15);

	return !(b00 == 4 && b01 == 3  && b02 == 8  && b03 == 7 && b04 == 12  && b05 == 11  && b06 == 16 && b07 == 15 && b08 == 20 && b09 == 19 && b10 == 24 && b11 == 23 && b12 == 28 && b13 == 27 && b14 == 32 && b15 == 31);
}
