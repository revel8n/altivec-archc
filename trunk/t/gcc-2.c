#include <altivec.h>

// use GCC intrisics to altivec instructions

// more details at:
// http://developer.apple.com/hardwaredrivers/ve/tutorial.html

int main()
{
	vector signed long v1 = (vector signed long) {-3, -1025, 2, 8}; 
	vector signed long v2 = (vector signed long) {1, 1, 3333, -1};
	vector signed long v3;
    signed long correct[4] = { -2, -1024, 3335, 7 };
	
	v3 = (vector long) vec_add(v1, v2);

	// FIXME: why doesn't work with a local variable?
    signed long *result = (signed long *) 0x1000;
    int i;

    for (i = 0; i < 4; i++) {
        vec_ste(v3, i, result);
        if (*result != correct[i]) { return 1; }
        result++;
    }
    
    return 0;
}
