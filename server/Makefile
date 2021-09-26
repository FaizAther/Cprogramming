CC=gcc
SRC=./src/
OBJ=./obj/
BIN=./bin/

TEST=./test/
TESTBIN=$(TEST)bin/

LDFLAGS=
CFLAGS=-Wall -Werror -Wextra -Wconversion -ggdb -g -pedantic -std=c11

EXE=main
SRCS=$(wildcard $(SRC)*.c)
BINS=$(BIN)$(EXE)
OBJS=$(patsubst $(SRC)%.c, $(OBJ)%.o, $(SRCS))

TESTS=$(wildcard $(TEST)*.c)
TESTBINS=$(patsubst $(TEST)%.c, $(TESTBIN)%, $(TESTS))

DIRS=$(BIN) $(OBJ)

.PHONY: all
all: $(DIRS) $(BINS)

$(BINS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

$(DIRS):
	mkdir $@

$(TESTBINS): $(TESTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -lcriterion $< -o $@

test: $(TESTBIN) $(TESTBINS)
	for test in $(TESTBINS); do ./$$test; done

$(TESTBIN):
	mkdir $@

.PHONY: clean
clean:
	rm -rvf $(DIRS) $(TESTBIN)
