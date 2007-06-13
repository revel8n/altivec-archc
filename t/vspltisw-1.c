#include "test.h"

int main() {
	uint32_t a, b, c, d;

	asm("vspltisw 15, -16");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (-16) &&
             b == (-16) &&
             c == (-16) &&
             d == (-16));
}
