#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210);
	LOAD_VECTOR_U(13, 0x00000008, 0x00000000, 0x00000000, 0x00000000);

	asm("vslo 14, 12, 13");

	STORE_VECTOR_U(14, a, b, c, d);

	return !(a == 0x23456700 && b == 0xabcdef01 &&  
		 c == 0xdcba9889 && d == 0x543210fe);
}

