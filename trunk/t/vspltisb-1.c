#include "test.h"

int main() {
	int8_t a, b, c, d;

	asm("vspltisb 15, -16");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (-16) &&
             b == (-16) &&
             c == (-16) &&
             d == (-16));
}
