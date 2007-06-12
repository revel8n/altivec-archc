#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00000001, 0xDEADBEEF, 0x11111111, 0x00020002);
	LOAD_VECTOR_U(14, 0x0000001F, 0x00000010, 0xFFFFFF02, 0x0000000B);

	asm("vrlw 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x80000000 && b == 0xBEEFDEAD &&
		 c == 0x44444444 && d == 0x10001000);
}
