#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01020408, 0x00FF0012, 0x1100FFFF, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x0102FFFE, 0x00FF0034, 0x11000001, 0x01010101);

	asm("vmulesh 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0xFFFFF7F0 &&
             b == 0x000003A8 &&
		     c == 0xFFFFFFFF &&
             d == 0xFFFFFEFF);
}
