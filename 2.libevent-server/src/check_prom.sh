#!/bin/sh

res0=`cat prom.log | grep "accept" | wc -l | sed 's/[[:space:]]//g'`
res1=`cat prom.log | grep "free" | wc -l | sed 's/[[:space:]]//g'`

echo $res0 $res1

if test $res0 = "45" -a $res1 = "45"
then
	echo "Prom passed"
	exit 0
else
	echo "Prom Failed"
	exit 1
fi
