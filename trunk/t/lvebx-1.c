#include "test.h"

int main() {

	uint8_t b00, b01, b02, b03, b04, b05, b06, b07, b08, b09, b10, b11, b12, b13, b14, b15;

	uint8_t *uv = (int8_t *) 0x1000;
	*uv = (int8_t) 42;

	asm("lvebx 13, 0, %0" : : "r"(uv));

	STORE_VECTOR_UBYTE(13, b00, b01, b02, b03, b04, b05, b06, b07, b08, b09, b10, b11, b12, b13, b14, b15);
	return !(b00 == 42);
}
