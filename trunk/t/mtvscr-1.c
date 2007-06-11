#include "test.h"

int main() {
    uint32_t b[4];
    uint32_t *uv = (uint32_t *) 0x1000;

    *uv = (uint32_t) 42; uv++;
    *uv = (uint32_t) 42; uv++;
    *uv = (uint32_t) 42; uv++;
    *uv = (uint32_t) 42;

    asm("lvewx 13, 0, %0" : : "r"(uv));
    asm("mtvscr 13");
    asm("mfvscr 23");

    STORE_VECTOR_U(23, b[3], b[2], b[1], b[0]);

    return !(b[3] == 42 && b[2] == 0 &&
             b[1] == 0  && b[0] == 0);
}
