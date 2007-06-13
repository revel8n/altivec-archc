#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0xDEADBEEF, 1, 2, 3);

	asm("vspltw 15, 13, 3");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0xDEADBEEF &&
             b == 0xDEADBEEF &&
             c == 0xDEADBEEF &&
             d == 0xDEADBEEF);
}
