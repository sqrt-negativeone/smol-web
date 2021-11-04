#!/bin/bash

code="$PWD"
opts=-g
cd build > /dev/null
g++ -o0 $opts $code/src/smol.c -o smol -pthread
cd $code > /dev/null
cp -r $code/static $code/build/
