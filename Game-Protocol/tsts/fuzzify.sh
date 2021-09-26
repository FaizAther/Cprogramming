#!/bin/bash
"$@"  | while read -r line; do 
    strip_mr=$(echo $line | awk -F: '{if ($1 == "MR") {printf "%s:%s:\n", $1, $2} else {printf "%s\n", $0}}')
    strip_res=$(echo $strip_mr | awk -F: '{if ($1 == "MATCH") {printf "%s:%s:%s:\n", $1,$2,$3} else {printf "%s\n", $0}}')
    echo $strip_res
done
exit "${PIPESTATUS[0]}"
