#include "test.h"

int main() {
	uint8_t a, b, c, d, e;

	LOAD_VECTOR_U(13, 0xaabbccdd, 0xeeffabcd, 0xacadaeaf, 0xbabcbdbe);

	asm("vspltb 15, 13, 3");
	// 0 ->  0xba; 
	// 3 ->  0xbe; 
	// 15 ->  0xdd; 
	e = 0xbe; 

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == e &&
             b == e &&
             c == e &&
             d == e);
}
