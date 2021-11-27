#!/bin/sh

curl -v http://localhost:3333/metrics > tests/out.$1.curl
cat tests/out.$1.curl | sed 's/[[:space:]][0-9]*//' > tests/out.$1.strip.curl
