#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x0102FFFE, 0xFFFFFF7F, 0x90810011, 0xFFFF7FFF);
	LOAD_VECTOR_U(14, 0x0201FEFF, 0x0001FE80, 0xAA7E0099, 0xFF0110FF);

	asm("vcmpgtsb 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00010100 && b == 0x00000101 &&
		     c == 0x00000001 && d == 0x00000100);
}
