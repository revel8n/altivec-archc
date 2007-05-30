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

	


