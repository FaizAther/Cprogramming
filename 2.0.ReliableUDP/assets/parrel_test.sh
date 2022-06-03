#!/bin/bash

pgrep RServer.exe
if test $? -ne 0
then
        ./RServer.exe 1> server.out 2> server.err &
fi

sleep 4

i=0
while test $i -lt 50
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done

sleep 4
ps

i=50
while test $i -lt 100
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done

sleep 4
ps

i=100
while test $i -lt 200
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done

sleep 4
ps

i=200
while test $i -lt 400
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done

sleep 4
ps

i=400
while test $i -lt 700
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done

ps
sleep 4

i=700
while test $i -lt 2000
do
        ./RClient.exe -n $i > logs/$i.client &
        echo $i
        i=$(($i + 1))
done