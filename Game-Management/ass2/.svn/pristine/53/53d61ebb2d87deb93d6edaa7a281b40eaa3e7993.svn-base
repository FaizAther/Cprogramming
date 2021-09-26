CC	:=gcc
CCFLAGS	:=-std=gnu99 -Wall -pedantic -g
LDFLAGS	:=

TARGETS	:= 2310hub 2310A 2310B
MAINS	:= $(addsuffix .o, $(TARGETS) )
OBJS	:= agent.o game.o grid.o move.o read.o check.o queue.o stack.o list.o node.o $(MAINS)
DEPS	:= agent.h game.h grid.h move.h read.h check.h queue.h stack.h list.h node.h

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJS)

$(OBJS): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJS)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)
