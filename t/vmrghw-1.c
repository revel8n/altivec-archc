#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xa00ab00b, 0xc00cd00d, 0xe00ef00f, 0xc0aae0bb);
	LOAD_VECTOR_U(13, 0x00a00030, 0x0f1111e0, 0x00000000, 0x00f000a0);

	asm("vmrghw 14, 12, 13");

	STORE_VECTOR_U(14, a, b, c, d);

	return !(a == 0x00000000 && b == 0xe00ef00f &&  
		 c == 0x00f000a0 && d == 0xc0aae0bb);
}

