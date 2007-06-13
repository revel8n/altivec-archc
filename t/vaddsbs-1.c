#include "test.h"

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0x0000007F, 0x00000000, 0x00000000, 0x00000000);
	LOAD_VECTOR_U(14, 0x00000001, 0x00000000, 0x00000000, 0x00000000);

	asm("vaddsbs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x0000007F && b == 0x00000000 &&
		 c == 0x00000000 && d == 0x00000000)) {
        return 1;
    }

    SATURATED(sat);

    if (!sat) {
        return 1;
    }

    MARK_NOT_SAT();
	
    LOAD_VECTOR_U(13, 0x0000007E, 0x00000080, 0x00000000, 0x00000000);
	LOAD_VECTOR_U(14, 0x00000001, 0x00000001, 0x00000000, 0x00000000);

	asm("vaddsbs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

    SATURATED(sat)

    if (sat) {
        return 1;
    }

    return 0;
}
