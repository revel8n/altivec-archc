#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFF0002, 0x7FFF8000, 0x7FFF8000, 0x12340010);
	LOAD_VECTOR_U(14, 0xFFFF0001, 0x00000000, 0x80007FFF, 0x12340100);

	asm("vcmpgtsh. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000001 && b == 0x00010000 &&
		     c == 0x00010000 && d == 0x00000000);
}
