CC	:=gcc
CCFLAGS	:=-pedantic -Wall -pthread -std=gnu99
LDFLAGS	:=

TARGETS	:= rpsclient rpsserver
MAINS	:= $(addsuffix .o, $(TARGETS) )
OBJS	:= $(MAINS)
DEPS	:=

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJS)

$(OBJS): %.o : %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAINS), $(OBJS)) %.o
	$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)