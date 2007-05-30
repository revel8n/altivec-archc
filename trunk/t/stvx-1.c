#include "test.h"

int main() {
	uint32_t a, b, c, d;
	uint32_t e, f, g, h;

	LOAD_VECTOR_U(13, 1, 10, 20, 30);
	STORE_VECTOR_U(13, a, b, c, d);

	LOAD_VECTOR_U(13, a, b, c, d);
	STORE_VECTOR_U(13, e, f, g, h);


	return !(a == 1 && b == 10 && c == 20 && d == 30 &&
		 e == a && f == b  && g == c  && h == d);
}
