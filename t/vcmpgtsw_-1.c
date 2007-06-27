#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0x0110FF0F, 0x80000000, 0x11111111);
	LOAD_VECTOR_U(14, 0xFFFFFF00, 0x0010FF0F, 0x7FFFFFFF, 0xFF0000FF);

	asm("vcmpgtsw. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000001 && b == 0x00000001 &&
		     c == 0x00000000 && d == 0x00000001);
}
