.PHONY: all

all: ch1.exe ch2.exe

ch1.exe: ch1.o
	gcc -o ch1.exe ch1.o -no-pie
ch1.o: ch1.asm
	nasm -f elf64 -g -F dwarf ch1.asm -l ch1.lst

ch2.exe: ch2.o
	gcc -o ch2.exe ch2.o -no-pie
ch2.o: ch2.asm
	nasm -f elf64 -g -F dwarf ch2.asm -l ch2.lst

.PHONY: clean

clean:
	rm -rf *.o *.exe *.lst