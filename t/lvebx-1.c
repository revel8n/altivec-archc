#include "test.h"

int main() {

	uint8_t *uv = (int8_t *) 0x1000;
	*uv = (int8_t) 42;

	asm("lvebx 
}
