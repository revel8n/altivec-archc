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

count: $(tests)
	 @make test | grep -v '^---$$' | grep -v '^     CC' |  sed -e 's/.* \(.*\)-.*/\1/' | uniq | grep -v gcc | wc -l

../model/powerpc.x:
	@echo Vai compilar o modelo :-P
	@false

test: $(tests) ../model/powerpc.x
	@echo ---
	@for i in $(bases); do ../model/powerpc.x --load=$$i.bin 2> /dev/null > /dev/null; echo $$?$$?$$? $$i; done

