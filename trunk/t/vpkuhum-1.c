#include "test.h"

int main() {
	uint32_t a, b, c, d;

	NEW_LOAD_VECTOR_U(13, 0x11FF00FF, 0x33FF22FF, 0x55FF44FF, 0x77FF66FF);
	NEW_LOAD_VECTOR_U(14, 0x99008800, 0xBB00AA00, 0xDD00CC00, 0xFF00EE00);

	asm("vpkuhum 15, 13, 14");

	NEW_STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x33221100 && b == 0x77665544 &&
             c == 0xBBAA9988 && d == 0xFFEEDDCC);
}
