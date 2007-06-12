#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x01010101, 0x10101010, 0x00FF00FF, 0x02020202);
	LOAD_VECTOR_U(14, 0x00010204, 0x00010204, 0x03033333, 0x00070811);

	asm("vrlb 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x01020410 && b == 0x10204001 &&
		 c == 0x00FF00FF && d == 0x02010204);
}
