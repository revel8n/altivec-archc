#include "test.h"

int main() {
	uint32_t a, b, c, d;

	NEW_LOAD_VECTOR_U(13, 0x00001111, 0x22223333, 0x44445555, 0x66667777);
	NEW_LOAD_VECTOR_U(14, 0x88889999, 0xAAAABBBB, 0xCCCCDDDD, 0xEEEEFFFF);

	asm("vpkuwum 15, 13, 14");

	NEW_STORE_VECTOR_U(15, a, b, c, d);

	return !(a == 0x22220000 && b == 0x66664444 &&
             c == 0xAAAA8888 && d == 0xEEEECCCC);
}
