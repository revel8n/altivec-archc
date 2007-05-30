#include "test.h"

int main() {
	uint8_t *ptr = (uint8_t *) 0x2000;
	uint8_t a;
	uint8_t b;

	LOAD_VECTOR_UWORD(13, 0, 0x0000CC00);

	asm("stvebx 13, 0, %0" : : "r"(0x2000) : "memory");
	asm("stvebx 13, 0, %0" : : "r"(0x2001) : "memory");
	a = *ptr;
	ptr++;
	b = *ptr;

	return !(a == 0x00 && b == 0xCC);
}
