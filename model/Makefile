all:
	acsim powerpc.ac -abi
	make -f Makefile.archc clean
	make -f Makefile.archc

debug:
	acsim powerpc.ac -abi
	make -f Makefile.archc clean
	make -f Makefile.archc DEBUG="-DDEBUG_MODEL -g"

clean:
	-make -f Makefile.archc clean model_clean
