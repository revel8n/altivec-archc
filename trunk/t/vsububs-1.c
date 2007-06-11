#include "test.h"

int main() {
	uint32_t a, b, c, d;


	LOAD_VECTOR_U(13, 0x00000000, 0xFE10FA0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x10001000, 0xCA10FF01, 0x11221234, 0x01020304);

	asm("vsububs 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x00000000 && b == 0x3400000E &&
		 c == 0xEEDD0000 && d == 0xFEFDFCFB);
}
