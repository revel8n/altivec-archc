#include "test.h"

// Make some saturated operation and check if it marked correctly

int main() {
	uint32_t a, b, c, d;
    uint32_t x[4];
    uint32_t *uv = (uint32_t *) 0x800;
    int sat;

	LOAD_VECTOR_U(13, 0xFFFFFFFF, 0xFE10FF0F, 0xFFFF0000, 0xFFFFFFFF);
	LOAD_VECTOR_U(14, 0x00000001, 0x00100001, 0x00001234, 0x00000000);

	asm("vaddubs 15, 13, 14");
   
    DIE_IF_NOT_SATURATED();

    // Clean SAT bit
    MARK_NOT_SAT();
    DIE_IF_SATURATED();

    return 0;
}

