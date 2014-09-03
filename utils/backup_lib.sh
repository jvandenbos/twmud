#!/bin/csh -ef

cd ../lib

find . \( -name '#*' -o -name '*~' -o -name '*.tmp' -o -name '*.bak' \) -print\
	| xargs rm -f

tar cvf - \
	*.messages actions ban.list bugs credits help help_table ideas \
	info messages motd news policy poses typos \
	players.d \
	tinyworld.{mob,obj,shp,wld,zon} \
	world \
	zone | \
	gzip > ../lib.tar.z
