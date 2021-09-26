CC=gcc
CFLAGS=-Wall -pedantic -std=gnu99 -g

.PHONY=all clean
.DEFAULT_GOAL:=all

DEPS=naval.o

all: naval

naval: $(DEPS)
	$(CC) $(CFLAGS) $(DEPS) -o naval

clean:
	rm -f *.o
	rm -f naval
