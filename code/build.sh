#!/bin/bash

code="$PWD"
defines='-DDEBUG_BUILD'
opts='-m32 -g  --all-warnings -Wall -Wextra -Wconversion  -Wuninitialized -Wtype-limits -Winline'
cd ../build > /dev/null
mkdir -p debug_reports
g++ $defines $opts $code/linux_main.cpp -o debugger

opts='-m32 -g  --all-warnings -Wall -Wextra -Wconversion -Wuseless-cast -Wuninitialized -Wtype-limits -Winline'
g++ $defines $opts $code/foo.cpp -o foo32

opts='-m64 -g  --all-warnings -Wall -Wextra -Wconversion -Wuseless-cast -Wuninitialized -Wtype-limits -Winline'
g++ $defines $opts $code/foo.cpp -o foo64

cd $code > /dev/null
