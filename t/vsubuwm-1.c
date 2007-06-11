#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 1, 10, 20, 30);

	LOAD_VECTOR_U(13, 2, 11, 21, 31);
	LOAD_VECTOR_U(14, 1,  1,  1,  1);

	asm("vsubuwm 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 1 && b == 10 && c == 20 && d == 30);
}
