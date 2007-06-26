#include "test.h"

int main() {
	uint32_t a, b, c, d, branched;

	LOAD_VECTOR_U(13, 0x00000000, 0x1110FE0F, 0xFEFE0000, 0xF10000F1);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFE11FF1F, 0xFFFF0101, 0xFFFFFFFF);

	asm("vcmpgtub. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_NOT_ALL_FALSE();
    
	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x00000000 && b == 0x00000000 &&
		  c == 0x00000000 && d == 0x00000000)) {
        return 1;
    }
    
    return 0;
}
