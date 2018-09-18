#!/bin/sh

while test $# -gt 0;do
	case "$1" in
		-d)
			shift 
			compile=$(find $1 -maxdepth 1 -name "*.c")
			shift
			;;
		-c)
			shift 
			options=$(echo $1)
			shift
			;;		
		*)
			break
			;;
	esac
done
if [ -z "$compile" ];
then
	for i in .; do
		compile=$(find $i -maxdepth 1 -name "*.c")
	done
fi
for j in $compile; do
	c=$(echo $j|sed -e 's/c$//' -e 's/.$//') 
	if [ -z "$1" ];
	then
		gcc -o $c $options $j
	else
		gcc -o $c $options $j 2>&1 | grep $1
	fi
done	
