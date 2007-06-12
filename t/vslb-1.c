#include "test.h"
#include "sample_data.h"

uint32_t sl_each_byte(uint32_t w, int sh)
{
    uint32_t result = 0;
    uint8_t b;
    int i;

    for (i = 0; i < 4; i++) {
        b = (uint8_t) (0x000000FF & (w >> (8 * i)));
        b <<= sh;
        result |= ((uint32_t) b & 0x000000FF) << (8 * i);
    }

    return result;
}

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W1, W11, W9, W2);
	LOAD_VECTOR_U(21, 0x01010101, 0x02020202, 0x03030303, 0x04040404);

	asm("vslb 22, 20, 21");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == sl_each_byte(W1, 1) &&
             b == sl_each_byte(W11, 2) &&
             c == sl_each_byte(W9, 3) &&
             d == sl_each_byte(W2, 4));
}
