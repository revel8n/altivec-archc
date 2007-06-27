#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x0102047F, 0x00FF00FF, 0x11002280, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x0102047F, 0x00FF0034, 0x11002280, 0x01010101);

	asm("vmulesb 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00043F01 &&
             b == 0x0001FFCC &&
		     c == 0x00004000 &&
             d == 0xFFFFFFFF);
}
