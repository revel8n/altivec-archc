#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 1, 10, 20, 30);
	LOAD_VECTOR_U(14, 1,  1,  1,  1);

	asm("vadduwm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 2 && b == 11 && c == 21 && d == 31);
}
