#include "test.h"

int main() {
	uint32_t a, b, c, d;
	
	LOAD_VECTOR_U(13, 0xEFEFFE00, 0x0010F00F, 0xF11F0000, 0x1F00001F);
    LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFE11FF1F, 0xFFFF0000, 0xFFFFFFFF);

	asm("vcmpgtuh. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_NOT_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000000 && b == 0x00000000 &&
		     c == 0x00000000 && d == 0x00000000);
}
