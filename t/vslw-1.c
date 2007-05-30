#include "test.h"
#include "sample_data.h"

// vslw

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W1, W11, W9, W2);
	LOAD_VECTOR_U(21, 1, 2, 3, 4);

	asm("vslw 22, 20, 21");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == (W1 << 1) && b == (W11 << 2) &&
		 c == (W9 << 3) && d == (W2 << 4));
}
