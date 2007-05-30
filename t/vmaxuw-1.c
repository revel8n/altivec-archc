#include "test.h"

int main() {
	uint32_t b00, b01, b02, b03;

	LOAD_VECTOR_U(13, 1, 3, 5, 7);
	LOAD_VECTOR_U(14, 4, 2, 8, 6);

	asm("vmaxuw 15, 13, 14");

	STORE_VECTOR_U(15, b00, b01, b02, b03);

	return !(b00 == 4 && b01 == 3  && b02 == 8  && b03 == 7);
}

