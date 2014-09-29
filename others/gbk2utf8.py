#!/usr/local/bin/python

import os
import sys


name = [""]
sufix = ["txt"]

def file_fliter(path) :
    if exsit_in_list(sufix, os.path.splitext(path)[1][1:]) :
        return True
    if exsit_in_list(name, os.path.basename(path)) :
        return True
    return False

def exsit_in_list(list, value) :
    for item in list :
        if item == value :
            return True
        return False

def convert(path, in_encode = "GBK", out_encode = "UTF8") :
    print path
    if file_fliter(path) :
        try :
            print "convert "
            content = open(path).read()
            new_content = content.decode(in_encode).encode(out_encode)
            open(path, "w").write(new_content)
            print "done"
        except :
            print "failed"
    else :
        print "ignore"

def explore(dir) :
    for root, dirs, files in os.walk(dir) :
        for file in files:
            path = os.path.join(root, file)
            convert(path)

def main() :
    for path in sys.argv[1:] :
        print path
        if os.path.isfile(path) :
            convert(path)
        elif os.path.isdir(path) :
            explore(path)

if __name__ == "__main__" :
    main()

