#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0xabcdefed, 0xbcdefedc, 0xcdefedcb, 0x0efedcba);

	asm("vupkhsb 14, 12");

	STORE_VECTOR_U(14, a, b, c, d);
	uint32_t m , n, o, p; 
	m =   0x000efffe;
	n =   0xffdcffba;
	o =   0xffcdffef;
	p =   0xffedffcb;  


	return !(a == p && b == o &&  
		 c == n && d == m);
}

