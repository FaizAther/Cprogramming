#!/bin/sh

i=0; while true; do curl -v http://localhost:27600/metrics > out.ip.$i.curl;out.ip.$i.curl; i="$(($i+1))"; done
