#include "test.h"

// Test saturation for half word unsigned add

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x11111234, 0x000FFFFF, 0x00000001, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x1111FFFF, 0x00010001, 0x0001FFFD, 0x00010001);

	asm("vadduhs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x2222FFFF && b == 0x0010FFFF &&
		 c == 0x0001FFFE && d == 0xFFFFFFFF);
}
