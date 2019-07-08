#!/bin/bash

code="$PWD"
defines='-DDEBUG_BUILD'
opts='-m64 -g -Wconversion -Wall -Wconversion'
cd ../build > /dev/null
mkdir -p debug_reports
g++ $defines $opts $code/linux_main.cpp -o debugger
g++ $defines $opts $code/foo.cpp -o foo
cd $code > /dev/null
