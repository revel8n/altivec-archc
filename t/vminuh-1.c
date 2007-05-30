#include "test.h"

int main() {
	uint16_t b00, b01, b02, b03, b04, b05, b06, b07;

	LOAD_VECTOR_UHWORD(13, 1, 3, 5, 7, 9, 11, 13, 15);
	LOAD_VECTOR_UHWORD(14, 4, 2, 8, 6, 12, 10, 16, 14);

	asm("vminuh 15, 13, 14");

	STORE_VECTOR_UHWORD(15, b00, b01, b02, b03, b04, b05, b06, b07);

	return !(b00 == 1 && b01 == 2  && b02 == 5  && b03 == 6 && b04 == 9  && b05 == 10  && b06 == 13 && b07 == 14);

}

