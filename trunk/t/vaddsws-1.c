#include "test.h"

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0x7FFFFFFF, 0x0, 0x0, 0x0);
	LOAD_VECTOR_U(14, 0x00000001, 0x0, 0x0, 0x0);

	asm("vaddsws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x7FFFFFFF && b == 0x0 &&
		 c == 0x0 && d == 0x00000000)) {
        return 1;
    }

    DIE_IF_NOT_SATURATED();
    MARK_NOT_SAT();
	
    LOAD_VECTOR_U(13, 0x7FFFFFF0, 0x80000000, 0x00000000, 0x00000000);
	LOAD_VECTOR_U(14, 0x0000000F, 0x00000001, 0x00000000, 0x00000000);

	asm("vaddsws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

    DIE_IF_SATURATED();

    return 0;
}
