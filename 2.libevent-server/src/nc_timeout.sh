#!/bin/sh

nc localhost 3333 > tests/out.$1.empty.nc &
sleep 30
