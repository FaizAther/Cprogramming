#!/bin/bash

FL0="../file.0.txt"
FL1="../file.1.txt"
FL2="../file.2.txt"
FL3="../file.n.txt"
FLN=$FL0

echo "ERRORS" > mismatched.ERR
cd logs
for FILE in `ls .`
do
	RUN_CMD=`head -1 $FILE`
	# echo $RUN_CMD
	TST_FLE=`echo $RUN_CMD |cut -d" " -f5`
	# echo $RUN_CMD
	FLX="../$FILE.$TST_FLE"
	# echo ../$FILE.$TST_FLE
	if test `echo $TST_FLE|cut -d"." -f2` = "0"
	then
		FLN=$FL0
	elif test `echo $TST_FLE|cut -d"." -f2` = "1"
	then
		FLN=$FL1
	elif test `echo $TST_FLE|cut -d"." -f2` = "2"
	then
		FLN=$FL2
	elif test `echo $TST_FLE|cut -d"." -f2` = "n"
	then
		FLN=$FL3
	fi
	# echo "diff $FLX $FLN 1>/dev/null 2>/dev/null"
	diff $FLX $FLN 1>/dev/null 2>/dev/null
	if test $? -eq 1
	then
		echo "FAILED: $RUN_CMD {diff $FLX $FLN}" >> ../mismatched.ERR
	fi
	# exit
done
cat ../mismatched.ERR |less