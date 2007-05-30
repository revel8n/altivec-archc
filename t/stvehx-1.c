#include "test.h"

int main() {
	uint16_t *ptr = (uint16_t *) 0x2002;
	uint16_t a;
	uint16_t b;

	LOAD_VECTOR_UWORD(13, 0, 0xDEAD0000);
	LOAD_VECTOR_UWORD(13, 1, 0x0000BEEF);

	asm("stvehx 13, 0, %0" : : "r"(0x2002) : "memory");
	asm("stvehx 13, 0, %0" : : "r"(0x2004) : "memory");
	a = *ptr;
	ptr++;
	b = *ptr;

	return !(a == 0xDEAD && b == 0xBEEF);
}
