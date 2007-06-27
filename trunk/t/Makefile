sources := $(shell echo *.c)
bases := $(basename $(sources)) 
tests := $(addsuffix .bin, $(bases)) 

all: $(tests)

%.bin: %.c
	@powerpc-elf-gcc -maltivec -O0 --specs=archc -o $@ $<
	@powerpc-elf-gcc -maltivec -O0 --specs=archc -S $<
	@echo '    ' CC $@

clean: 
	-rm -f *.bin *.s

count: $(tests) ../model/powerpc.x
	 @make -s do_test | sed -e 's/.* \(.*\)-.*/\1/' | sort | uniq | grep -v gcc | wc -l

../model/powerpc.x:
	@echo Vai compilar o modelo :-P
	@false

do_test: $(tests) ../model/powerpc.x
	@for i in $(bases); do ../model/powerpc.x --load=$$i.bin 2> /dev/null > /dev/null; echo $$?$$?$$? $$i; done

test: $(tests) ../model/powerpc.x
	@make -s do_test > /tmp/altivec.tests.txt
	@cat /tmp/altivec.tests.txt | grep -v '^000' | cat
	@echo ---
	@echo -n 'passed: '
	@cat /tmp/altivec.tests.txt | grep '^000' | wc -l
	@echo -n ' total: '
	@cat /tmp/altivec.tests.txt | wc -l
	@echo -n 'unique: '
	@cat /tmp/altivec.tests.txt | sed -e 's/.* \(.*\)-.*/\1/' | sort | uniq | grep -v gcc | wc -l 
	@rm -f /tmp/altivec.tests.txt

