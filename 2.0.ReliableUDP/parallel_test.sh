#!/bin/bash

pgrep RUSHBSvr
if test $? -ne 0
then
	echo Here
        ./RUSHBSvr 1> server.out 2> server.err &
fi

sleep 4
p=`head -1 server.out`
echo $p

i=0
./multiple_test.sh $p $i 50

sleep 4
ps

i=50
./multiple_test.sh $p $i 100

sleep 4
ps

i=100
./multiple_test.sh $p $i 200

sleep 4
ps

i=200
./multiple_test.sh $p $i 400

sleep 4
ps

i=400
./multiple_test.sh $p $i 700

ps
sleep 4

i=700
./multiple_test.sh $p $i 2000

i=2000
./multiple_test.sh $p $i 4000

ps
exit
