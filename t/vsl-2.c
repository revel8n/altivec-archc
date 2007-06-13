#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210);
	LOAD_VECTOR_U(13, 0x04040404, 0x04040404, 0x04040404, 0x04040404);

	asm("vsl 14, 12, 13");

	STORE_VECTOR_U(14, a, b, c, d);

	return !(a == 0x12345670 && b == 0x9ABCDEF0 &&  
		 c == 0xEDCBA988 && d == 0x6543210F  );
}

