#include "test.h"
#include "sample_data.h"

uint32_t sra_each_half(uint32_t w, int sh)
{
    uint32_t result = 0;
    int16_t hh;
    uint16_t h;
    int i;

    for (i = 0; i < 2; i++) {
        hh = (int16_t) (0x0000FFFF & (w >> (16 * i)));
        hh >>= sh;
        h = (uint16_t) hh;
        result |= ((uint32_t) h & 0x0000FFFF) <<  (16 * i);
    }

    return result;
}

int main() {
	uint32_t a, b, c, d;

	LOAD_VECTOR_U(20, W1, W11, W9, W2);
	LOAD_VECTOR_U(21, 0x00010001, 0x00020002, 0x00030003, 0x00040004);

	asm("vsrah 22, 20, 21");

	STORE_VECTOR_U(22, a, b, c, d);

	return !(a == sra_each_half(W1, 1) &&
             b == sra_each_half(W11, 2) &&
             c == sra_each_half(W9, 3) &&
             d == sra_each_half(W2, 4));
}
