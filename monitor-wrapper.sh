#!/bin/bash

PATH1='/dev/'
PATH2='/cdrom'
PATH3='/proc'

DIRNAME='LabSOMod2Canale1'

find / -maxdepth 3 -type d \( -path $PATH1 -o -path $PATH2 -o -path $PATH3 \) -prune -o -name $DIRNAME -print 2> /dev/null | 
while read LINE ;
do
	cd "$LINE"
	if [ -f monitor ]; then
    		echo "Il monitor è stato trovato in $LINE e sarà avviato."
		./monitor&
		exit 0
	else 
    		echo "Il monitor non è stato trovato in $LINE."
	fi
done
exit 0

