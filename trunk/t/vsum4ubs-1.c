#include "test.h"

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0x01010101, 0x01010101, 0xFFFFFFFF, 0x0);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0x00000001, 0x00000000, 0x1);

	asm("vsum4ubs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0xFFFFFFFF && b == 0x5 &&
		  c == 0x000003FC && d == 0x1)) {
        return 1;
    }

    SATURATED(sat);

    if (!sat) {
        return 1;
    }

    MARK_NOT_SAT();
	
    LOAD_VECTOR_U(13, 0xFFFFFFFF, 0x0, 0x01010101, 0x00000001);
	LOAD_VECTOR_U(14, 0x00000000, 0x0, 0xFFFFFFFB, 0x00000FF);

	asm("vsum4ubs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x000003FC && b == 0x0 &&
          c == 0xFFFFFFFF && d == 0x00000100)) {
        return 1;
    }

    SATURATED(sat)

    if (sat) {
        return 1;
    }

    return 0;
}
