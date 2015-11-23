#!/bin/bash

if [ $# -eq 0 ]; then
    exit 0
elif [ "$1" == "all" ]; then
	cd ..;
   	make -s;
   	cd example;
    filelist=`ls *.cpp`
    echo compile :$filelist
    for file in $filelist ; do
        echo compiling $file ...
        g++ -std=c++11 -DCOMPONET_DEBUG -fPIC -ggdb -pthread -o test_${file%%.*} $file -L.. -lcomponet -lrt -Wl,-rpath,.. 
    done
elif [ "$1" == "clean" ]; then
    rm -f *.o *.d *.d.* test_* *.log*
else
	cd ..;
   	make -s;
   	cd example;
    file=$1
    g++ -std=c++11 -DCOMPONET_DEBUG -fPIC -ggdb -pthread -o test_${file%%.*} $1 -L.. -lcomponet -lrt -Wl,-rpath,.. 
fi
