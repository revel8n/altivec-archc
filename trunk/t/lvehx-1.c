#include "test.h"

int main() {

	uint16_t b00, b01, b02, b03, b04, b05, b06, b07;

	uint16_t *uv = (uint16_t *) 0x1000;
	*uv = (uint16_t) 42;

	asm("lvehx 13, 0, %0" : : "r"(uv));

	STORE_VECTOR_UHWORD(13, b07, b06, b05, b04, b03, b02, b01, b00); 
	return !(b00 == 42);
}
