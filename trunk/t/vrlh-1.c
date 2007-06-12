#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00010001, 0x1111FFFF, 0x00010001, 0x00020002);
	LOAD_VECTOR_U(14, 0x00000010, 0x0001000F, 0x0001000F, 0x00070004);

	asm("vrlh 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00010001 && b == 0x2222FFFF &&
		 c == 0x00028000 && d == 0x01000020);
}
