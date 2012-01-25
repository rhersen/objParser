#!/bin/sh
while [ true ]
do inotifywait cunit.c
    gcc -g obj2gl.c -lcunit && ./a.out | spc --config=spcrc
done
