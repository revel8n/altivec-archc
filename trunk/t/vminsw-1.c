#include "test.h"

int main() {
	int32_t b00, b01, b02, b03;

	LOAD_VECTOR_S(13, 1, 3, 5, 7);
	LOAD_VECTOR_S(14, 4, 2, 8, 6);

	asm("vminsw 15, 13, 14");

	STORE_VECTOR_S(15, b00, b01, b02, b03);

	return !(b00 == 1 && b01 == 2  && b02 == 5  && b03 == 6);

}

