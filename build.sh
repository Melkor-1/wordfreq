#!/usr/bin/sh

set -xe

RELEASE_CFLAGS="-O2 -s"
DEBUG_CFLAGS="-g3 -ggdb"

gcc -std=c2x $RELEASE_CFLAGS new.c -o new 
time ./new < kjvbible_10.txt | wc -l

