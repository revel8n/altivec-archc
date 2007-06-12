#include <altivec.h>

// use GCC intrisics to altivec instructions

// more details at:
// http://developer.apple.com/hardwaredrivers/ve/tutorial.html

int main()
{
	vector unsigned long v1 = (vector unsigned long) {2, 7, 2, 8}; 
	vector unsigned long v2 = (vector unsigned long) {1, 3, 1, 1};
	vector unsigned long v3;
    long correct[4] = { 1, 4, 1, 7 };

	v3 = (vector unsigned long) vec_sub(v1, v2);

	// FIXME: why doesn't work with a local variable?
	unsigned long *result = (unsigned long *) 0x1000;
    int i;

    for (i = 0; i < 4; i++) {
        vec_ste(v3, i, result);
        if (*result != correct[i]) { return 1; }
        result++;
    }
    
    return 0;
}
