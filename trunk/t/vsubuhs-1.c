#include "test.h"

// Test saturation for half word unsigned add

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x22221234, 0x00100000, 0x0000F001, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x11111243, 0x0001FFFF, 0x0001FFFD, 0x00010002);

	asm("vsubuhs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x11110000 && b == 0x000F0000 &&
		 c == 0x00000000 && d == 0xFFFEFFFD);
}
