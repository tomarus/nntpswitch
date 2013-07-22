#!/bin/sh

VERSION=0.12

if [ -d .svn ]; then
    svnversion -cn .|sed s/.*://g|sed s/exported//g
elif [ -d CVS ]; then
    DATE=`date +%Y%m%d`
    printf "%s" "$VERSION-CVS-$DATE"
else
    echo $VERSION
fi
