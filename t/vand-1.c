#include "test.h"
#include "sample_data.h"

// vand

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W1, W11, W9, W2);
	LOAD_VECTOR_U(21, W8, WF,  W3, W7);

	asm("vand 22, 21, 20");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == (W1 & W8) && b == (W11 & WF) &&
		 c == (W9 & W3) && d == (W2 & W7));
}
