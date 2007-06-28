#include "test.h"

int main() {
	uint32_t a, b, c, d;

	NEW_LOAD_VECTOR_U(13, 0x00110000, 0x00330022, 0x01550144, 0x00770066);
	NEW_LOAD_VECTOR_U(14, 0x00990088, 0x00BB00AA, 0xFFFFFFFF, 0x8000CCCC);

	asm("vpkshss 15, 13, 14");
    
    DIE_IF_NOT_SATURATED();

	NEW_STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x33221100 && b == 0x77667F7F &&
             c == 0x7F7F7F7F && d == 0x8080FFFF);
}
