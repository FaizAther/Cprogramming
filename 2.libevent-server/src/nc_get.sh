#!/bin/sh

(printf "GET /metrics"; sleep 2; printf "HTTP/1.1\n\n"; sleep 1)| nc localhost 3333 > tests/out.$1.nc
cat tests/out.$1.nc | sed 's/[[:space:]][0-9]*//' > tests/out.$1.strip.nc
