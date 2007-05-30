#include "test.h"

int main() {

	uint32_t b00, b01, b02, b03;

	uint32_t *uv = (uint32_t *) 0x1000;
	*uv = (uint32_t) 42;

	asm("lvewx 13, 0, %0" : : "r"(uv));

	STORE_VECTOR_U(13, b03, b02, b01, b00); 
	return !(b00 == 42);
}
