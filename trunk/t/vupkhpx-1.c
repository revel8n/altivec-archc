#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xabcdefed, 0xbcdefedc, 0xcdefedcb, 0x0efedcba);

	asm("vupkhpx 14, 12");

	STORE_VECTOR_U(14, a, b, c, d);
	uint32_t m , n, o, p; 
	m = 0x0003171e;
	n = 0xff130f0f;
	o = 0xff0f061e; 
	p = 0xff0a1e0d;      

	return !(a == p && b == o &&  
		 c == n && d == m);
}

