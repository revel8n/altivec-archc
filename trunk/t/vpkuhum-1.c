#include "test.h"

int main() {
	uint32_t a, b, c, d;

	NEW_LOAD_VECTOR_U(13, 0xFF11FF00, 0xFF33FF22, 0xFF55FF44, 0xFF77FF66);
	NEW_LOAD_VECTOR_U(14, 0x00990088, 0x00BB00AA, 0x00DD00CC, 0x00FF00EE);

	asm("vpkuhum 15, 13, 14");

	NEW_STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x33221100 && b == 0x77665544 &&
             c == 0xBBAA9988 && d == 0xFFEEDDCC);
}
