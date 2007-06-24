#include "test.h"

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0xFFFFFFF0, 0x0, 0x11111111, 0x0);
	LOAD_VECTOR_U(14, 0x80000000, 0x1, 0x7FFFFFFF, 0x1);

	asm("vsum2sws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x80000000 && b == 0x0 &&
		 c == 0x7FFFFFFF && d == 0x0)) {
        return 1;
    }

    SATURATED(sat);

    if (!sat) {
        return 1;
    }

    return 0;

    MARK_NOT_SAT();
	
    LOAD_VECTOR_U(13, 0x7FFFFFFF, 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0x99999999, 0x00000002, 0x99999999);

	asm("vsum2sws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

    SATURATED(sat)

    if (sat) {
        return 1;
    }

    return 0;
}
