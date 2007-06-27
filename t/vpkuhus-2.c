#include "test.h"

int main() {
	uint32_t a, b, c, d;
	
    NEW_LOAD_VECTOR_U(13, 0x0011FFFF, 0x00330022, 0x00550044, 0x00770066);
	NEW_LOAD_VECTOR_U(14, 0x00990088, 0x00BB00AA, 0x00DD00CC, 0x00FF00EE);

	asm("vpkuhus 15, 13, 14");
    
    DIE_IF_NOT_SATURATED();

	NEW_STORE_VECTOR_U(15, a, b, c, d);
	
    return !(a == 0x332211FF && b == 0x77665544 &&
             c == 0xBBAA9988 && d == 0xFFEEDDCC);
}
