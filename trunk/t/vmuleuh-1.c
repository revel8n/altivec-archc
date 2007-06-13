#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01020408, 0x00FF0012, 0x11002200, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x01020408, 0x00FF0034, 0x11002222, 0x01010101);

	asm("vmuleuh 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00104040 &&
             b == 0x000003A8 &&
		     c == 0x04888400 &&
             d == 0x0100FEFF);
}
