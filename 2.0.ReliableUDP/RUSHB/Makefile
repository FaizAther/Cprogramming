.PHONY: clean run debug all

all: RUSHBSvr# RUSHBCnt

RUSHBSvr:
	make -C ../
	cp ../RUSHBSvr .

RUSHBCnt: RUSHBSvr
	cp ../RClient.exe .

run: clean RUSHBSvr
	./RUSHBSvr -p5001

debug: clean RUSHBSvr
	gdb ./RUSHBSvr -q

test: clean RUSHBSvr
	./tst.sh

clean:
	make -C ../ clean
	rm RUSHBSvr RClient.exe
