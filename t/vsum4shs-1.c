#include "test.h"

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0x00010001, 0x00010001, 0xFFFFFFFF, 0x0);
	LOAD_VECTOR_U(14, 0x7FFFFFFF, 0x00000001, 0x00000002, 0x1);

	asm("vsum4shs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x7FFFFFFF && b == 0x3 &&
		 c == 0x0 && d == 0x1)) {
        return 1;
    }

    DIE_IF_NOT_SATURATED();

    MARK_NOT_SAT();
	
    LOAD_VECTOR_U(13, 0xFFFFFFFF, 0x0, 0xFFFFFFFF, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0x0, 0x0, 0x0000002);

	asm("vsum4shs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0xFFFFFFFD && b == 0x0 &&
		 c == 0xFFFFFFFE && d == 0x0)) {
        return 1;
    }
    
    DIE_IF_SATURATED();

    return 0;
}
