#include "test.h"
#include "sample_data.h"

int main() {
	uint32_t a, b, c, d, branched;

	LOAD_VECTOR_U(13, W1, W2, W3, W4);
	LOAD_VECTOR_U(14, W1, W2, W3, W4);

	asm("vcmpequh. 15, 13, 14");

    DIE_IF_NOT_ALL_TRUE();
    DIE_IF_ALL_FALSE();
    
	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0x00010001 && b == 0x00010001 &&
		  c == 0x00010001 && d == 0x00010001)) {
        return 1;
    }

    return 0;
}
