#!/bin/bash

echo $1

make && ./test$1.exec -s"127.0.0.1" -uroot  
