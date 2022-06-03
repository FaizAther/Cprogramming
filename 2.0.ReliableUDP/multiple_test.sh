#!/bin/bash

if test $# -ne 3
then
	exit
fi

if test $2 -gt $3
then
	exit
fi

j=0
RUN_I=$2
CHKSUM="-c"
ENC="-e"
MISB="-m"
while test $RUN_I -lt $3
do
	if test $j -eq 0
	then
		j=1
		CHKSUM=""
		ENC=""
	elif test $j -eq 1
	then
		j=2
		CHKSUM="-c"
		ENC=""
		MISB=""
	elif test $j -eq 2
	then
		j=3
		CHKSUM=""
		ENC="-e"
		MISB=""
	elif test $j -eq 3
	then
		j=4
		CHKSUM=""
		ENC=""
		MISB="-m"
	elif test $j -eq 4
	then
		j=5
		CHKSUM=""
		ENC="-e"
		MISB="-m"
	elif test $j -eq 5
	then
		j=6
		CHKSUM="-c"
		ENC="-e"
		MISB="-m"
	else
		j=0
		CHKSUM="-c"
		ENC="-e"
		MISB=""
	fi
	echo "./single_test.sh $RUN_I $CHKSUM $ENC $MISB $1"
	./single_test.sh $RUN_I $CHKSUM $ENC $MISB $1
	RUN_I=`expr $RUN_I + 1`
done
