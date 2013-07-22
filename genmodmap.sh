#!/bin/bash
## $Id: genmodmap.sh,v 1.4 2006-09-07 19:32:03 mjo Exp $

for f in "$@"; do
	case $f in
	*auth_*.c) AUTHS="$AUTHS $f" ;;
	*acct_*.c) ACCTS="$ACCTS $f" ;; 
	## must skip the rest; see Makefile
	esac
done

echo "/* Automatically generated on `date` */"

## authmap

echo "struct { char *name; void (*ptr)(); } authmap[] = {"
for f in $AUTHS; do
	FN=`basename $f | sed 's/\.c//'`
	echo -e "\t{\"$FN.so\", $FN},"
done
echo "};"
echo

## acctmap

echo "struct { char *name; void (*ptr)(); } acctmap[] = {"
for f in $ACCTS; do
	FN=`basename $f | sed 's/\.c//'`
	echo -e "\t{\"$FN.so\", $FN},"
done
echo "};"
echo
