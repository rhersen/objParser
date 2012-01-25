#!/bin/sh
while [ true ]
do inotifywait *.c
    gcc -g obj2gl.c -lcunit && ./a.out | spc --config=spcrc
done
