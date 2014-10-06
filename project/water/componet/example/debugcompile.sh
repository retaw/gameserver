#!/bin/bash

if [ $# -eq 0 ]; then
    exit 0
elif [ "$1" == "all" ]; then
    echo compile all
    filelist=`ls *.cpp`
    for file in $filelist ; do
        echo compiling $file ...
        g++ -std=c++11 -fPIC -ggdb -pthread -o test_${file%%.*} $file -L.. -lbase -lrt -Wl,-rpath,.. 
    done
elif [ "$1" == "clean" ]; then
    rm -f *.o *.d *.d.* test_* *.log*
else
    file=$1
    g++ -std=c++11 -fPIC -ggdb -pthread -o test_${file%%.*} $1 -L.. -lbase -lrt -Wl,-rpath,.. 
fi
