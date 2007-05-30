#include <altivec.h>

// use GCC intrisics to altivec instructions

// more details at:
// http://developer.apple.com/hardwaredrivers/ve/tutorial.html

int main()
{
	vector unsigned long v1 = (vector unsigned long) {2, 0, 2, 8}; 
	vector unsigned long v2 = (vector unsigned long) {1, 1, 1, 1};
	vector unsigned long v3;
	
	v3 = (vector unsigned long) vec_add(v1, v2);

	// FIXME: why doesn't work with a local variable?
	unsigned long *result = (unsigned long *) 0x1000;
	
	vec_ste(v3, 0, result);

	return !(*result == 3);
}
