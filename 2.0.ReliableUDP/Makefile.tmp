CXX=gcc
CXXFLAGS=-Wall -Wextra -pedantic# -g -ggdb -DDEBUG -DDEBUG_END
LDD=-std=c99 -pthread

BINS=RUSHBSvr

SRCS=$(wildcard *.c)
DEPS=$(wildcard *.h)

.PHONY: all clean run debug tar update
all: ${BINS}

RUSHBSvr: RUSHBSvr.o
	$(CXX) $(CXXFLAGS) $(LDD) $< -o $@

RUSHBSvr.o: RUSHBSvr.c ${DEPS}
	$(CXX) $(CXXFLAGS) $(LDD) $< -c

run: all
	./RUSHBSvr & 1>server.out 2>server.err 

debug: all
	gdb ./${BINS} -q

dir:
	rm -rf logs
	mkdir logs

tar: clean dir
	cp -r ${SRCS} ${DEPS} Makefile testing rushbsvr.log.txt submit
	tar cvf `date +%F.%H.%Z`_assn2.tar.gz submit
	rm -rf submit

test: clean all dir
	./parallel_test.sh

validate: test
	./verify_test.sh && ./kill_rserver.sh

update: clean
	git add -A && git commit -am"`date`" && git push

clean:
	rm -rf ${BINS} *.o *_assn2.tar.gz outtesting submit logs/* *.client.* mismatched.ERR
