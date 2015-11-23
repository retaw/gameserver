#!/bin/bash

path=~/server_release

rm -rf $path
mkdir $path

cp --parents -rf processes/*/*_exec $path
cp --parents -rf water/libs/*/installed $path
cp --parents -rf  protocol/rawmsg/public/*.xml $path
cp -rf config $path
cp -rf script $path
cp -f ./*.sh $path
cp -f ./*.py $path

#cd ~/std_svn/公共/server/config/
#svn up
#cd -
#cp -rf ~/std_svn/公共/server/config/ $path
#cp -f process.xml.me $path/config/process.xml
mkdir $path/log
