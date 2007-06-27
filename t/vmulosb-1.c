#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01027F00, 0x00FFFF00, 0x11008000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x01027F00, 0x00FF3400, 0x11002200, 0x01010101);

	asm("vmulosb 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00013F01 &&
             b == 0x0000FFCC &&
		     c == 0x0121EF00 &&
             d == 0xFFFFFFFF);
}
