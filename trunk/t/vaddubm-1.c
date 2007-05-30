#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01000000, 0x0000000F, 0x00FF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x10000000, 0x00000001, 0x00020000, 0x01010101);

	asm("vaddubm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x11000000 && b == 0x00000010 &&
		 c == 0x00010000 && d == 0x00000000);
}
