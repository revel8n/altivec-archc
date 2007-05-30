#include "test.h"
#include "sample_data.h"

// vandc

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, WF, W2, W3, W5);
	LOAD_VECTOR_U(21, W0, W7, WF, W6);

	asm("vandc 22, 20, 21");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == (WF & ~W0) && b == (W2 & ~W7) &&
		 c == (W3 & ~WF) && d == (W5 & ~W6));
}
