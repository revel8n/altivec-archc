#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xcdefedcb, 0x0efedcba, 0xabcdefed, 0xbcdefedc);

	asm("vupklpx 14, 12");

	STORE_VECTOR_U(14, a, b, c, d);
	uint32_t m , n, o, p; 
	m = 0x0003171e;
	n = 0xff17051a; 
	o = 0xff130f0f; 
	p = 0xff1b0e0b; 

	return !(a == p && b == o &&  
		 c == n && d == m);
}

