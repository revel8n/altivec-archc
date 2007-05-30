#include "test.h"

// Should saturate for each word

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00000001, 0x00FF00FF, 0xFF00FF00, 0x0F0F0F0D);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFF00FF02, 0x00FF00FF, 0xF0F0F0F1);

	asm("vadduws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0xFFFFFFFF && b == 0xFFFFFFFF &&
		 c == 0xFFFFFFFF && d == 0xFFFFFFFE);
}
