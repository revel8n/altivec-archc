#include "test.h"

int main() {
	int16_t a, b, c, d;

	asm("vspltish 15, -1");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (-1) &&
             b == (-1) &&
             c == (-1) &&
             d == (-1));
}
