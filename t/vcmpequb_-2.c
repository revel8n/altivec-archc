#include "test.h"
#include "sample_data.h"

int main() {
	uint32_t a, b, c, d, branched;

	LOAD_VECTOR_U(13, W1, W2, W3, W4);
	LOAD_VECTOR_U(14, W1, W2, W3, W4);

	asm("vcmpequb. 15, 13, 14");

    DIE_IF_NOT_ALL_TRUE();
    DIE_IF_ALL_FALSE();
    
	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x01010101 && b == 0x01010101 &&
		  c == 0x01010101 && d == 0x01010101)) {
        return 1;
    }

    return 0;
}
