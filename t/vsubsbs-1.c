#include "test.h"

// Should saturate for each word

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xF0070001, 0xF0000001, 0xF0000001, 0xF0000001);
	LOAD_VECTOR_U(14, 0x7F807F80, 0x7FFF7FFF, 0x7FFF7FFF, 0x7FFF7FFF);

	asm("vsubsbs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x807f817f && b == 0x80018102 &&
		 c == 0x80018102 && d == 0x80018102);
}
