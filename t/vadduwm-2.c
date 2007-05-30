#include "test.h"

// Should calculate modulo for each word

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00000001, 0x00FF00FF, 0xFF00FF03, 0x0F0F0F0F);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFF00FF02, 0x00FF00FF, 0xF0F0F0F4);

	asm("vadduwm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0 && b == 1 && c == 2 && d == 3);
}
