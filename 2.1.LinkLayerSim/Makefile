CC=gcc
CCFLAGS=-Wall -Wextra -pedantic -g -ggdb# -std=c99
LDD=-lm

DEPS=$(wildcard *.h)

.PHONY: all clean

all: RUSHBSwitch

RUSHBSwitch: RUSHBSwitch.c ${DEPS}
	${CC} ${CCFLAGS} RUSHBSwitch.c -o RUSHBSwitch ${LDD}

run: clean all
	./main

debug: clean all
	gdb ./main -q

update: all clean
	git add -A; git commit -m"`date` modified"; git push

clean:
	rm RUSHBSwitch
