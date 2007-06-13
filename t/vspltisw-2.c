#include "test.h"

int main() {
	uint32_t a, b, c, d;

	asm("vspltisw 15, 11");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (11) &&
             b == (11) &&
             c == (11) &&
             d == (11));
}
