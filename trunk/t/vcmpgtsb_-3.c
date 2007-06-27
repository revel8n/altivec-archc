#include "test.h"

int main() {
	uint32_t a, b, c, d;
	
	LOAD_VECTOR_U(13, 0xFF01FEFB, 0xFEF1FE80, 0x80800099, 0xFE0110FF);
    LOAD_VECTOR_U(14, 0x0102FFFE, 0xFFFFFF7F, 0x90810111, 0xFF0F7F0F);

	asm("vcmpgtsb. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_NOT_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000000 && b == 0x00000000 &&
		     c == 0x00000000 && d == 0x00000000);
}
