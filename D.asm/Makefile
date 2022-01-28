EXE=$(patsubst %.asm,%,$(wildcard *.asm))

.PHONY: all clean

all: $(EXE)

%: %.o
	ld -m elf_i386 $< -o $@

%.o: %.asm
	nasm -f elf32 $< -o $@

clean:
	rm -rf *.o $(EXE)
