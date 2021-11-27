CC      :=  gcc
CCFLAGS :=  -std=gnu99 -Wall -pedantic -g

.PHONY: all clean

all: scanner

clean:
	rm -f *.o scanner

scanner:
	gcc ${CCFLAGS} scanner.c lex.yy.c -o scanner
