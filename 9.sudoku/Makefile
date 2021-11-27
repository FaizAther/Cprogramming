CC=gcc
BINS=main
HEADS=sudoku solve
BINS_HEADS=$(BINS) $(HEADS)
OBJS=$(addsuffix .o, $(BINS_HEADS))
DEPS=$(addsuffix .h, $(HEADS))
LDFLAGS=-pthread -ggdb

.PHONY: all
all: $(OBJS) $(BINS)

%: %.o
	gcc -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	gcc -c $< $(DEPS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rvf $(BINS) $(OBJS) $(addsuffix .gch, $(DEPS))
