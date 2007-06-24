#include "test.h"

int main() {
	uint16_t a, b, c, d, e;

	LOAD_VECTOR_U(13, 0xababCDCD, 0xdeadbeef, 0xdcdcefef, 0xfefebaba);

	asm("vsplth 15, 13, 7");
	// 0 ->  0xfefe; 
	// 3 ->  0xefef; 
	// 7 ->  0xcdcd; 
	e = 0xcdcd; 

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == e &&
             b == e &&
             c == e &&
             d == e);
}
