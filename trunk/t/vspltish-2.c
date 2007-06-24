#include "test.h"

int main() {
	int16_t a, b, c, d;

	asm("vspltish 15, 15");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (15) &&
             b == (15) &&
             c == (15) &&
             d == (15));
}
