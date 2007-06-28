#include "test.h"

int main() {
	uint32_t a, b, c, d;

	NEW_LOAD_VECTOR_U(13, 0x00001111, 0x22223333, 0x44445555, 0x66667777);
	NEW_LOAD_VECTOR_U(14, 0x88889999, 0xAAAABBBB, 0xCCCCDDDD, 0xEEEEFFFF);

	asm("vpkuwum 15, 13, 14");

	NEW_STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x33331111 && b == 0x77775555 &&
             c == 0xBBBB9999 && d == 0xFFFFDDDD);
}
