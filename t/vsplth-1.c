#include "test.h"

int main() {
	uint16_t a, b, c, d, e;

	LOAD_VECTOR_U(13, 0xababcdcd, 0xdeadbeef, 0xdcdcEFEF, 0xfefebaba);

	asm("vsplth 15, 13, 3");
	// 0 ->  0xfefe; 
	// 3 ->  0xefef; 
	// 7 ->  0xcdcd; 
	e = 0xefef; 

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == e &&
             b == e &&
             c == e &&
             d == e);
}
