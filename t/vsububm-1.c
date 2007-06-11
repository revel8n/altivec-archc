#include "test.h"

int main() {
	uint32_t a, b, c, d;


	LOAD_VECTOR_U(13, 0x11000000, 0x00000010, 0x00010000, 0x00000000);
	LOAD_VECTOR_U(14, 0x10000000, 0x00000001, 0x00020000, 0x01010101);

	asm("vsububm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x01000000 && b == 0x0000000F &&
		 c == 0x00FF0000 && d == 0xFFFFFFFF);
}
