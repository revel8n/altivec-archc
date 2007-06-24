#include "test.h"

int main() {
	int8_t a, b, c, d;

	asm("vspltisb 15, 15");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == (15) &&
             b == (15) &&
             c == (15) &&
             d == (15));
}
