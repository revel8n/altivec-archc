#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFFFF00, 0x0010FF0F, 0xFFFF0000, 0xFF0000FF);

	asm("vcmpequh. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x00010000 && b == 0x00000001 &&
		  c == 0x00010001 && d == 0x00000000)) {
        return 1;
    }

    return 0;
}
