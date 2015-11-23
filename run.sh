#!/bin/bash


DYNAMIC_LIB_DIRS=$(mysql_config --variable=pkglibdir):$(pwd)/water/libs/mysql++/installed/lib

chomd +x ./start.h
export LD_LIBRARY_PATH=$DYNAMIC_LIB_DIRS && ./start.sh
