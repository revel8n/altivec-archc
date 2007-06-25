#include "test.h"

int main() {

	/*vra.data[0] e vrb.data[0]*/
	int *b = (int *) 2000;
	int vec_a[] = {0x10000001, 0xffff0001, 0xff010100, 0x00018000};
	int vec_b[] = {0x0010ff02, 0xffff0001, 0xff020000, 0xffff0102};
	int vec_c[] = {0x7ffffffe, 0xffff7fff, 0xfffe7fff, 0x80000000};

	*b = vec_a[0];
	asm( "li 2, 2000; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[0];
	asm( "li 2, 2000; "
	    "lvewx 3, 0, 2; ");
	*b = vec_c[0];
	asm( "li 2, 2000; "
	    "lvewx 4, 0, 2; ");

	/*vra.data[1] e vrb.data[1]*/
	b = (int *) 2004;
	*b = vec_a[1];
	asm( "li 2, 2004; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[1];
	asm( "li 2, 2004; "
	    "lvewx 3, 0, 2; ");
	*b = vec_c[1];
	asm( "li 2, 2004; "
	    "lvewx 4, 0, 2; ");

	/*vra.data[2] e vrb.data[2]*/
	b = (int *) 2008;
	*b = vec_a[2];
	asm( "li 2, 2008; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[2];
	asm( "li 2, 2008; "
	    "lvewx 3, 0, 2; ");
	*b = vec_c[2];
	asm( "li 2, 2008; "
	    "lvewx 4, 0, 2; ");

	/*vra.data[3] e vrb.data[3]*/
	b = (int *) 2012;
	*b = vec_a[3];
	asm( "li 2, 2012; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[3];
	asm( "li 2, 2012; "
	    "lvewx 3, 0, 2; ");
	*b = vec_c[3];
	asm( "li 2, 2012; "
	    "lvewx 4, 0, 2; ");


	asm("vmhraddshs 1, 2, 3, 4; ");

        uint32_t i, j, k, l;
        uint32_t m, n, o, p;
        STORE_VECTOR_U(1, l, k, j, i);

        m = 0x7ffffffe;
        n = 0xffff7fff;
        o = 0xffff7fff;
        p = 0x8000ff7f;    

        return !(i == m && j == n && k == o && l == p);


}
