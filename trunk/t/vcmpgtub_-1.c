#include "test.h"

int main() {
	uint32_t a, b, c, d, branched;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0xFFFFFF00, 0x0010FF0F, 0xFFFF0000, 0xFF0000FF);

	asm("vcmpgtub. 15, 13, 14");

    DIE_IF_ALL_TRUE();
    DIE_IF_ALL_FALSE();
    
	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x00000001 && b == 0x01000000 &&
		  c == 0x00000000 && d == 0x00010100)) {
        return 1;
    }
    
    return 0;
}
