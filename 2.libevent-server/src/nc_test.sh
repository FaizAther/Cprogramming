#!/bin/sh

j=0; while true; do (printf "GET /metrics"; sleep 2; printf "HTTP/1.1\n\n"; sleep 1)| nc localhost 27600 > out.$i.nc; i="$(($i+1))"; done
