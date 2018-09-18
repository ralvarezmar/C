#!/bin/sh

if [ $# -ne 4 ]
then
	echo "usage: $1 $2 $3 $4" 1>&2
	exit 1
fi
nombres=`awk '{print $1}'$* | grep -v "#" | sort -u`

echo "#notas\tEjer1\tEjer2\tEjer3\tEjer4\tFinal" >> notasfinales.txt
for i in $nombres
do
	echo -n "$i\t" >> notasfinales.txt
	nflag=n
	for j in $*
	do
		notaparcial=`cat $j | egrep $i | awk '{print $2}'`
		if [ -z $notaparcial ]
		then
			echo -n "-\t" >> notasfinales.txt
			nflag=y
		else
			echo -n "$notaparcial\t" >> notasfinales.txt
		fi
	done
	if test $nflag = n ; then
		notas=`cat $* | egrep $i | awk '{print $2}'`
		media=`echo $notas | awk '{print $1 + $2 + $3 + $4}'`
		media=`echo "scale=2; $media/4" | bc -l`
		echo $media >> notasfinales.txt
	else
		echo NP >> notasfinales.txt
	fi
done
