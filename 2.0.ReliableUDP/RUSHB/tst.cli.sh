#!/bin/bash

TESTS_TIMEOUT=",INVALID_CHECKSUM_FLAG,TIMEOUT,MULTI_TIMEOUT"

TESTS=`echo "SIMPLE,NAK,MULTI_NAK,INVALID_ACK,INVALID_SEQ,INVALID_FLAGS,CHECKSUM,INVALID_CHECKSUM_VAL$TESTS_TIMEOUT" | sed "s/,/\n/g"`
if test $# -gt 0
then
	for T in $TESTS
	do
		echo $T
		python3 RUSHBSampleClient.py 0 $1 -m $T
	done
else
	for T in $TESTS
	do
		echo $T
		python3 RUSHBSampleTest.py $T
	done
fi
