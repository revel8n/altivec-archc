#include "test.h"
#include "sample_data.h"

// vxor

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W11, W2, W3, W4);
	LOAD_VECTOR_U(21, W5, W7, W7, W8);

	asm("vxor 22, 21, 20");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == (W11 ^ W5) && b == (W2 ^ W7) &&
		 c == (W3 ^ W7) && d == (W4 ^ W8));
}
