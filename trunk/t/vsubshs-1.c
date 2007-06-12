#include "test.h"

// Should saturate for each word

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xF0000001, 0x7F000001, 0x7FFF00B2, 0xF0000001);
	LOAD_VECTOR_U(14, 0x7FFF8000, 0x7FFF8002, 0x7F00000F, 0x7FFF7FFF);

	asm("vsubshs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x80007FFF && b == 0xFF017FFF &&
		 c == 0x00FF00A3 && d == 0x80008002);
}
