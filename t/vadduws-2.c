#include "test.h"

// Check saturation behaviour

int main() {
	uint32_t a, b, c, d;
    int sat;
	
    LOAD_VECTOR_U(13, 0x00000001, 0x00FF00FF, 0xFF00FF00, 0x0F0F0F0D);
	LOAD_VECTOR_U(14, 0xFFFFFFFF, 0xFF00FF02, 0x00FF00FF, 0xF0F0F0F1);

	asm("vadduws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

	if (!(a == 0xFFFFFFFF && b == 0xFFFFFFFF &&
		 c == 0xFFFFFFFF && d == 0xFFFFFFFE)) {
        return 1;
    }

    DIE_IF_NOT_SATURATED();
    
    MARK_NOT_SAT();
    
    LOAD_VECTOR_U(13, 0x00000001, 0x00FF00FF, 0xFF00FF00, 0x0F0F0F0D);
	LOAD_VECTOR_U(14, 0xFFFFFFFA, 0xFF00FF00, 0x00FF00FF, 0xF0F0F0F1);

	asm("vadduws 15, 13, 14");

	STORE_VECTOR_U(15, a, b, c, d);

    DIE_IF_SATURATED();

    return 0;
}
