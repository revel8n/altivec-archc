#include "test.h"

// Check if unsigned word-wise equality works, w/ CR6 flagging

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFFFF00, 0x0010FF0F, 0xFFFF0000, 0xFF0000FF);

	asm("vcmpequw. 15, 13, 14");

    // TODO: test if CR6 was correctly flagged (actually must be 0x00)


	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x00000000 && b == 0x00000000 &&
		  c == 0x00000001 && d == 0x00000000)) {
        return 1;
    }

    // TODO: test the two possible flags

    return 0;
}
