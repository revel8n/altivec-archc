#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xa00ab00b, 0xc00cd00d, 0xe00ef00f, 0xc0aae0bb);
	//0x0000-0030 almost-last nibble of vrb = 3 bytes to shift right. 
	LOAD_VECTOR_U(13, 0x00a00030, 0x00000000, 0x00000000, 0x00f000a0);
	//LOAD_VECTOR_U(13, 0x00100000, 0x00000000, 0x00000000, 0x00000000);

	asm("vsro 14, 12, 13");

	STORE_VECTOR_U(14, a, b, c, d);

	return !(a == 0x0cd00da0 && b == 0x0ef00fc0 &&  
		 c == 0xaae0bbe0 && d == 0x000000c0);
}

