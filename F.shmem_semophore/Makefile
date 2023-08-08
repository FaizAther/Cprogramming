CC=gcc
CFLAGS=-ggdb -g -pedantic -lpthread

.PHONY: all clean

all: create read destroy shared.mem

shared.mem:
	touch shared.mem

create: create.c shared.o
	${CC} ${CFLAGS} -o create create.c shared.o

read: read.c shared.o
	${CC} ${CFLAGS} -o read read.c shared.o

destroy: destroy.c shared.o
	${CC} ${CFLAGS} -o destroy destroy.c shared.o

clean:
	rm -rf read create destroy shared.o shared.mem
