#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01020408, 0x00FF0012, 0x1100FFFF, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFF0000, 0x00FF0034, 0x11000001, 0x01010101);

	asm("vmulosh 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0xFFFFFEFE &&
             b == 0x0000FE01 &&
		     c == 0x01210000 &&
             d == 0xFFFFFEFF);
}
