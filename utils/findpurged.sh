#!/bin/csh -f

cd $LIB

foreach file ( purged-*.gz )
	echo $file
	gzip -dc $file | tar tvf - | fgrep -i $1
end
