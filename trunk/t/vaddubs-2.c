#include "test.h"

// Check saturation behaviour

int main() {
	uint32_t a, b, c, d;
    int sat;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x10001000, 0x00100001, 0x11221234, 0x01020304);

	asm("vaddubs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0xFFFFFFFF && b == 0xFE20FF10 &&
		 c == 0xFFFF1234 && d == 0xFFFFFFFF)) {
        return 1;
    }

    SATURATED(sat);

    if (!sat) {
        return 1;
    }

    MARK_NOT_SAT();

   	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x00000000, 0x00000000, 0x00001234, 0x00000000);

	asm("vaddubs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

    SATURATED(sat)

    if (sat) {
        return 1;
    }

    return 0;
}
