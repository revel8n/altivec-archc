#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xcdefedcb, 0x0efedcba, 0xabcdefed, 0xbcdefedc);

	asm("vupklsh 14, 12");

	STORE_VECTOR_U(14, a, b, c, d);
	uint32_t m , n, o, p; 
	m =  0x00000efe;
	n =  0xffffdcba;
	o =  0xffffcdef;
	p =  0xffffedcb;      


	return !(a == p && b == o &&  
		 c == n && d == m);
}

