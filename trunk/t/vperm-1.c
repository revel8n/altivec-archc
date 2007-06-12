#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(12, 0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210);
	LOAD_VECTOR_U(13, 0xFEDCBA98, 0x76543210, 0x01234567, 0x89ABCDEF);
	// 16, 27, 8, 3; 4, 11, 17, 0; 25, 9, 30, 12; 5, 1, 20, 29.
	LOAD_VECTOR_U(14, 0x101B0803, 0x040B1100, 0x19091E0C, 0x0501141D);

	asm("vperm 15, 12, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x89108910 && b == 0xfeefab76 &&  
		 c == 0x54abba01 && d == 0xdc5401dc  );
}

