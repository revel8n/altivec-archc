int main() {

	/*vra.data[0] e vrb.data[0]*/
	int *b = (int *) 2000;
	int vec_a[] = {0xff, 0x1, 0xf0, 0}; 
	int vec_b[] = {0,   0xf1, 0x0f, 0}; 
	*b = vec_a[0];
	asm( "li 2, 2000; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[0];
	asm( "li 2, 2000; "
	    "lvewx 3, 0, 2; ");

	/*vra.data[1] e vrb.data[1]*/
	b = (int *) 2004;
	*b = vec_a[1];
	asm( "li 2, 2004; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[1];
	asm( "li 2, 2004; "
	    "lvewx 3, 0, 2; ");

	/*vra.data[2] e vrb.data[2]*/
	b = (int *) 2008;
	*b = vec_a[2];
	asm( "li 2, 2008; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[2];
	asm( "li 2, 2008; "
	    "lvewx 3, 0, 2; ");

	/*vra.data[3] e vrb.data[3]*/
	b = (int *) 2012;
	*b = vec_a[3];
	asm( "li 2, 2012; "
	    "lvewx 2, 0, 2; ");
	*b = vec_b[3];
	asm( "li 2, 2012; "
	    "lvewx 3, 0, 2; ");


	asm("vmaxuw  1, 2, 3; ");
	//asm("vmaxsw  1, 2, 3; ");


	return *b;
}
/*
	asm("li 3, 0; "
	    "li 2, 2000; "
	    "lvewx 2, 3, 2; ");

	int base = 2000, step=4, q=4, 
	    vec_a[] = {10,  73, -12, 0}, 
	    vec_b[] = { 1, -73,   2, 0}; 
	str ad[256]; 
	for (int i=0; i < q; i++){
		sprintf(ad, "li 2, %d; \n"
		            "lvewx 2, 0, 2; \n", base + i*step);
	}
*/
