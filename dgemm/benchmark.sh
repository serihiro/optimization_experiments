#!/bin/bash

gcc -c util.c -o util -std=c99
for source_code in `ls -a ./*.c | grep -v util.c`
do
    echo "target: ${source_code}"
    gcc $source_code util -Wall -o target -O1 -std=c99
    ./target
done
rm -rf ./target ./util
