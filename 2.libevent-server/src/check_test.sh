#!/bin/sh

i=0
while test $i -lt 15 
do
	diff out.base.strip.nc tests/out.$i.strip.nc > /dev/null
	if test $? -eq 1
	then
		exit 1
	fi
    
        diff out.base.empty.nc tests/out.$i.empty.nc > /dev/null
	if test $? -eq 1
	then
		exit 1
	fi

	diff out.base.strip.curl tests/out.$i.strip.curl > /dev/null
	if test $? -eq 1
	then
		exit 1
	fi
	i="$((i+1))"
done

