#include "test.h"

int main() {

	uint8_t b00, b01, b02, b03, b04, b05, b06, b07, b08, b09, b10, b11, b12, b13, b14, b15;

	uint8_t *uv = (uint8_t *) 0x1000;
	*uv = (uint8_t) 42;

	asm("lvebx 13, 0, %0" : : "r"(uv));

	STORE_VECTOR_UBYTE(13, b15, b14, b13, b12, b11, b10, b09, b08, b07, b06, b05, b04, b03, b02, b01, b00); 
	return !(b00 == 42);
}
