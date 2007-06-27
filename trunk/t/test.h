/* Header with helper functions for testing. */

typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;

typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;

#define LOAD_VECTOR_UWORD(vec, word_pos, data) { \
	uint32_t *uword = (uint32_t *) 0x1000; \
	uword += word_pos; \
	*uword = (uint32_t) data; \
	asm("lvewx " #vec ", 0, %0" : : "r" (uword) : "2"); \
	}

#define STORE_VECTOR_UWORD(vec, word_pos, data) { \
	uint32_t *uword = (uint32_t *) 0x2000; \
	uword += word_pos; \
	asm("stvewx " #vec ", 0, %0" : : "r" (uword) : "memory"); \
	data = *uword; \
	}

#define LOAD_VECTOR_U(vec, d3, d2, d1, d0) { \
	uint32_t *uv = (uint32_t *) 0x1000; \
	*uv = (uint32_t) d0; \
	uv++; \
	*uv = (uint32_t) d1; \
	uv++; \
	*uv = (uint32_t) d2; \
	uv++; \
	*uv = (uint32_t) d3; \
	uv -= 3; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
	}

#define STORE_VECTOR_U(vec, d3, d2, d1, d0) { \
	uint32_t *uv = (uint32_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	}

#define LOAD_VECTOR_UBYTE(vec, d15, d14, d13, d12, d11, d10, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) { \
	uint8_t *uv = (uint8_t *) 0x1000; \
	*uv = (uint8_t) d0; \
	uv++; \
	*uv = (uint8_t) d1; \
	uv++; \
	*uv = (uint8_t) d2; \
	uv++; \
	*uv = (uint8_t) d3; \
	uv++; \
	*uv = (uint8_t) d4; \
	uv++; \
	*uv = (uint8_t) d5; \
	uv++; \
	*uv = (uint8_t) d6; \
	uv++; \
	*uv = (uint8_t) d7; \
	uv++; \
	*uv = (uint8_t) d8; \
	uv++; \
	*uv = (uint8_t) d9; \
	uv++; \
	*uv = (uint8_t) d10; \
	uv++; \
	*uv = (uint8_t) d11; \
	uv++; \
	*uv = (uint8_t) d12; \
	uv++; \
	*uv = (uint8_t) d13; \
	uv++; \
	*uv = (uint8_t) d14; \
	uv++; \
	*uv = (uint8_t) d15; \
	uv -= 15; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
}

#define STORE_VECTOR_UBYTE(vec, d15, d14, d13, d12, d11, d10, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) { \
	uint8_t *uv = (uint8_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	uv++; \
	d4 = *uv; \
	uv++; \
	d5 = *uv; \
	uv++; \
	d6 = *uv; \
	uv++; \
	d7 = *uv; \
	uv++; \
	d8 = *uv; \
	uv++; \
	d9 = *uv; \
	uv++; \
	d10 = *uv; \
	uv++; \
	d11 = *uv; \
	uv++; \
	d12 = *uv; \
	uv++; \
	d13 = *uv; \
	uv++; \
	d14 = *uv; \
	uv++; \
	d15 = *uv; \
}

#define LOAD_VECTOR_UHWORD(vec, d7, d6, d5, d4, d3, d2, d1, d0) { \
	uint16_t *uv = (uint16_t *) 0x1000; \
	*uv = (uint16_t) d0; \
	uv++; \
	*uv = (uint16_t) d1; \
	uv++; \
	*uv = (uint16_t) d2; \
	uv++; \
	*uv = (uint16_t) d3; \
	uv++; \
	*uv = (uint16_t) d4; \
	uv++; \
	*uv = (uint16_t) d5; \
	uv++; \
	*uv = (uint16_t) d6; \
	uv++; \
	*uv = (uint16_t) d7; \
	uv++; \
	uv -= 7; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
}

#define STORE_VECTOR_UHWORD(vec, d7, d6, d5, d4, d3, d2, d1, d0) { \
	uint16_t *uv = (uint16_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	uv++; \
	d4 = *uv; \
	uv++; \
	d5 = *uv; \
	uv++; \
	d6 = *uv; \
	uv++; \
	d7 = *uv; \
}

// ss

#define LOAD_VECTOR_S(vec, d3, d2, d1, d0) { \
	int32_t *uv = (int32_t *) 0x1000; \
	*uv = (int32_t) d0; \
	uv++; \
	*uv = (int32_t) d1; \
	uv++; \
	*uv = (int32_t) d2; \
	uv++; \
	*uv = (int32_t) d3; \
	uv -= 3; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
	}

#define STORE_VECTOR_S(vec, d3, d2, d1, d0) { \
	int32_t *uv = (int32_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	}

#define LOAD_VECTOR_SBYTE(vec, d15, d14, d13, d12, d11, d10, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) { \
	int8_t *uv = (int8_t *) 0x1000; \
	*uv = (int8_t) d0; \
	uv++; \
	*uv = (int8_t) d1; \
	uv++; \
	*uv = (int8_t) d2; \
	uv++; \
	*uv = (int8_t) d3; \
	uv++; \
	*uv = (int8_t) d4; \
	uv++; \
	*uv = (int8_t) d5; \
	uv++; \
	*uv = (int8_t) d6; \
	uv++; \
	*uv = (int8_t) d7; \
	uv++; \
	*uv = (int8_t) d8; \
	uv++; \
	*uv = (int8_t) d9; \
	uv++; \
	*uv = (int8_t) d10; \
	uv++; \
	*uv = (int8_t) d11; \
	uv++; \
	*uv = (int8_t) d12; \
	uv++; \
	*uv = (int8_t) d13; \
	uv++; \
	*uv = (int8_t) d14; \
	uv++; \
	*uv = (int8_t) d15; \
	uv -= 15; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
}

#define STORE_VECTOR_SBYTE(vec, d15, d14, d13, d12, d11, d10, d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) { \
	int8_t *uv = (int8_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	uv++; \
	d4 = *uv; \
	uv++; \
	d5 = *uv; \
	uv++; \
	d6 = *uv; \
	uv++; \
	d7 = *uv; \
	uv++; \
	d8 = *uv; \
	uv++; \
	d9 = *uv; \
	uv++; \
	d10 = *uv; \
	uv++; \
	d11 = *uv; \
	uv++; \
	d12 = *uv; \
	uv++; \
	d13 = *uv; \
	uv++; \
	d14 = *uv; \
	uv++; \
	d15 = *uv; \
}

#define LOAD_VECTOR_SHWORD(vec, d7, d6, d5, d4, d3, d2, d1, d0) { \
	int16_t *uv = (int16_t *) 0x1000; \
	*uv = (int16_t) d0; \
	uv++; \
	*uv = (int16_t) d1; \
	uv++; \
	*uv = (int16_t) d2; \
	uv++; \
	*uv = (int16_t) d3; \
	uv++; \
	*uv = (int16_t) d4; \
	uv++; \
	*uv = (int16_t) d5; \
	uv++; \
	*uv = (int16_t) d6; \
	uv++; \
	*uv = (int16_t) d7; \
	uv++; \
	uv -= 7; \
	asm("lvx " #vec ", 0, %0" : : "r" (uv)); \
}

#define STORE_VECTOR_SHWORD(vec, d7, d6, d5, d4, d3, d2, d1, d0) { \
	int16_t *uv = (int16_t *) 0x2000; \
	asm("stvx " #vec", 0, %0" : : "r" (uv) : "memory"); \
	d0 = *uv; \
	uv++; \
	d1 = *uv; \
	uv++; \
	d2 = *uv; \
	uv++; \
	d3 = *uv; \
	uv++; \
	d4 = *uv; \
	uv++; \
	d5 = *uv; \
	uv++; \
	d6 = *uv; \
	uv++; \
	d7 = *uv; \
}

/* SATURATED() is deprecated, please use DIE_IF_SATURATED and
 * DIE_IF_NOT_SATURATED() */

/*
#define SATURATED(var) { \
    asm("mfvscr 30"); \
    uint32_t x[4]; \
    STORE_VECTOR_U(30, x[3], x[2], x[1], x[0]); \
    var = x[3] & 0x00000001; \
}
*/

#define MARK_NOT_SAT() { \
    LOAD_VECTOR_UWORD(30, 3, 0); \
    asm("mtvscr 30"); \
}

#define DIE_IF_SATURATED() { \
    asm("mfvscr 30"); \
    uint32_t x[4]; \
    STORE_VECTOR_U(30, x[3], x[2], x[1], x[0]); \
    if (x[3] & 0x00000001) {\
        return 1; \
    } \
}

#define DIE_IF_NOT_SATURATED() { \
    asm("mfvscr 30"); \
    uint32_t x[4]; \
    STORE_VECTOR_U(30, x[3], x[2], x[1], x[0]); \
    if (!(x[3] & 0x00000001)) {\
        return 1; \
    } \
}


#define DIE_IF_ALL_TRUE() { \
    int it_branched; \
    asm("bc 12, 24, 12 \n" \
        "li %0, 0 \n" \
        "b 8 \n" \
        "li %0, 1 \n" \
        : "=r" (it_branched) : : "0"); \
    if (it_branched) { \
        return 1; \
    } \
}

#define DIE_IF_NOT_ALL_TRUE() { \
    int it_branched; \
    asm("bc 12, 24, 12 \n" \
        "li %0, 0 \n" \
        "b 8 \n" \
        "li %0, 1 \n" \
        : "=r" (it_branched) : : "0"); \
    if (!it_branched) { \
        return 1; \
    } \
}

#define DIE_IF_ALL_FALSE() { \
    int it_branched; \
    asm("bc 12, 26, 12 \n" \
        "li %0, 0 \n" \
        "b 8 \n" \
        "li %0, 1 \n" \
        : "=r" (it_branched) : : "0"); \
    if (it_branched) { \
        return 1; \
    } \
}

#define DIE_IF_NOT_ALL_FALSE() { \
    int it_branched; \
    asm("bc 12, 26, 12 \n" \
        "li %0, 0 \n" \
        "b 8 \n" \
        "li %0, 1 \n" \
        : "=r" (it_branched) : : "0"); \
    if (!it_branched) { \
        return 1; \
    } \
}

