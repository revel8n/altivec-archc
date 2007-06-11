#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x11111234, 0x000F000F, 0x00000001, 0xFFFFFFFF);

	LOAD_VECTOR_U(13, 0x22221234, 0x00100010 , 0x00010001 , 0x00000000);
	LOAD_VECTOR_U(14, 0x11110000, 0x00010001, 0x00010000, 0x00010001);

	asm("vsubuhm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x11111234 && b == 0x000F000F &&
		 c == 0x00000001 && d == 0xFFFFFFFF);
}
