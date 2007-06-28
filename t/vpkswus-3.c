#include "test.h"

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(13, 0x00003333, 0x00004444, 0x00001111, 0x00002222);
	LOAD_VECTOR_U(14, 0xFFFF7777, 0x00008888, 0x00005555, 0x00006666);

	asm("vpkswus 15, 13, 14");
    
    DIE_IF_NOT_SATURATED();

	STORE_VECTOR_U(15, d, c, b, a);

	return !(a == 0x11112222 && b == 0x33334444 &&
             c == 0x55556666 && d == 0x00008888);
}
