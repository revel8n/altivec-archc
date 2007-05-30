int main() {
	register int a = 3;
	int *b = (int *) 2000;

	*b = 42;

	asm("li 3, 0; "
	    "li 2, 2000; "
	    "lvewx 1, 3, 2; ");

	asm("li 3, 0; "
	    "li 2, 3000; "
	    "stvewx 1, 3, 2; " : : : "memory");

	asm("li 3, 0;"
	    "li 2, 3000; "
	    "lvewx 2, 3, 2; ");

	b = (int *) 3000;

	return *b;
}
