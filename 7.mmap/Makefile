BINS=01-allocating-memory 02-reading-files 03-ipc 04-exec 04-payload 04-payload-bin
OBJS=04-payload.o

.PHONY: all
all: $(BINS)

%-bin: %.asm
	nasm -o $@ $<

%: %.o
	ld -o $@ $<

%.o: %.asm
	nasm -o $@ -felf64 $< 

%: %.c
	gcc -Wall -ggdb -o $@ $<

.PHONY: clean
clean:
	rm -rfv $(BINS) $(OBJS)
