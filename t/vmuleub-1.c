#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01020408, 0x00FF0012, 0x11002200, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x01020408, 0x00FF0034, 0x11002222, 0x01010101);

	asm("vmuleub 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00040040 &&
             b == 0xFE0103A8 &&
		     c == 0x00000000 &&
             d == 0x00FF00FF);
}
