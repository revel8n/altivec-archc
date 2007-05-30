#include "test.h"

int main() {
	int16_t b00, b01, b02, b03, b04, b05, b06, b07;

	LOAD_VECTOR_SHWORD(13, 1, 3, 5, 7, 9, 11, 13, 15);
	LOAD_VECTOR_SHWORD(14, 4, 2, 8, 6, 12, 10, 16, 14);

	asm("vmaxsh 15, 13, 14");

	STORE_VECTOR_SHWORD(15, b00, b01, b02, b03, b04, b05, b06, b07);

	return !(b00 == 4 && b01 == 3  && b02 == 8  && b03 == 7 && b04 == 12  && b05 == 11  && b06 == 16 && b07 == 15);
}

