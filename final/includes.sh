#!/bin/sh

  if [ -d $i ];
  then
    find $* -name "*.c" | xargs egrep "#include" | awk '{print $2}' | sort | uniq
  else
    echo "$i es argumento no válido "
  fi


