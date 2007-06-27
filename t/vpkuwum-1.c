#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00001111, 0x22223333, 0x44445555, 0x66667777);
	LOAD_VECTOR_U(14, 0x88889999, 0xAAAABBBB, 0xCCCCDDDD, 0xEEEEFFFF);

	asm("vpkuwum 15, 13, 14");

	STORE_VECTOR_U(15, d, c, b, a);

	return !(a == 0x44446666 && b == 0x00002222 &&
             c == 0xCCCCEEEE && d == 0x8888AAAA);
}
