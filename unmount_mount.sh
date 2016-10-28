#!/bin/bash
fusermount -u ./demo
./build.sh
./hello_single demo
> logging
gcc -Wall -o test writeread.c
