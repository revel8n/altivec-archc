#include "test.h"

int main() {
	uint32_t a;
	uint32_t b;

	LOAD_VECTOR_U(13, 1, 10, 20, 30);
	STORE_VECTOR_UWORD(13, 3, a);
	STORE_VECTOR_UWORD(13, 2, b);

	return !((a+b) == 11);
}
