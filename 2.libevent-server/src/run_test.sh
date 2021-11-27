#!/bin/sh

make -j4
rm prom.log
rm -rf tests >/dev/null
mkdir tests

if test $# -eq 1
then
	tar -xvf base.local.tar
else
	tar -xvf base.tar
fi

./prom-exporter -f -l prom.log -p 3333 &
sleep 2

i=0
while test $i -lt 15 
do
	./nc_get.sh $i &
	./curl_get.sh $i 1>/dev/null 2>/dev/null &
        ./nc_timeout.sh $i &
	i="$(($i+1))"
done
top -g "prom" > top.out
sleep 10
echo "Checking now"
./check_test.sh
if test $? -eq 0
then
	echo "Passed netcat"
	e=0
else
	echo "Failed netcat"
	e=1
fi

if test $# -eq 1
then
	ps -aux | grep "prom"| grep -v "grep"| sed 's/ [ ]*/ /g'| cut -d' ' -f2| xargs kill -INT
else
	ps -aux | grep "s4648481"| grep "prom"|grep -v "grep"| sed 's/ [ ]*/ /g'| cut -d' ' -f2| xargs kill -INT
fi

echo "kill prom then sleep 30"
sleep 30

if test $e -eq 0
then
	./check_prom.sh
	e=$?
fi

exit $e
