#include "test.h"

// Check if byte-wise saturation for add works

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x10001000, 0x00100001, 0x11221234, 0x01020304);

	asm("vaddubs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0xFFFFFFFF && b == 0xFE20FF10 &&
		 c == 0xFFFF1234 && d == 0xFFFFFFFF);
}
