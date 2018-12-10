#!/bin/bash
for i in `ls $(readlink -m $0/../tests)`
do
	echo -n Running test $i...
	if ! ./test --test $(readlink -m $0/../tests)/$i
	then
		echo failure!
		exit 1
	fi
	echo \ success!
done
 
