#include "test.h"
#include "sample_data.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W1, W11, W9, W2);
	LOAD_VECTOR_U(21, 1, 2, 3, 4);

	asm("vsraw 22, 20, 21");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == (((int32_t) W1) >> 1)); /* &&
             b == (((int32_t) W11) >> 2) &&
             c == (((int32_t) W9) >> 3) &&
             d == (((int32_t) W2) >> 4));*/
}
