#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFFFF00, 0x0010FF0F, 0x00FF0000, 0x000000FF);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFE10FF0F, 0x0FFF0000, 0x0FFFFFFF);

	asm("vcmpgtuw. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_NOT_ALL_FALSE();

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000000 && b == 0x00000000 &&
		     c == 0x00000000 && d == 0x00000000);
}
