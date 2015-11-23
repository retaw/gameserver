#!/bin/bash
svnst='svn st | grep -P "^(?!.*\.).|\.sql$|\.h$|\.cpp$|\.xml\.example$|\.sh$|^Makefile$|\.py$|.proto$" | grep -v -P "tmp|touchfile|\bcodedef\b|\.pb\.|installed|tags|_exec$|\.me$|test_.*$|makefile|config_me"'
#| egrep -v ".xml$|.temp$|log/|protobuf-2\.5\.0|\.codedef\.|_exec$|\.d\.|\.d$|\.pb\." | sed "s/^/\t&/g"'
#svnst='svn st | egrep  ".h$|.cpp%|.xml.example|.sh|^Makefile$" | egrep -v ".xml$|.temp$|log/|protobuf-2\.5\.0|\.codedef\.|_exec$|\.d\.|\.d$|\.pb\." | sed "s/^/\t&/g"'

filelist=$(eval $svnst)
if [[ -z $filelist ]]; then #是空串
    exit 0
fi

echo svn st:
eval $svnst
echo ""

if [[ $# == 0 ]]; then
    exit 0
fi


cut_files_column="$svnst | awk '{print \$2}'"
files=$(eval $cut_files_column)

svncmd="svn -m\"$1\" ci $files"
echo ""
echo "do u want commit these files? "
echo ""
echo "    "$svncmd
echo ""
read -p "[y/n]?" sure

if [[ $sure == Y || $sure == y ]] ; then
    eval $svncmd
fi
