#!/bin/csh -ef

cd $LIB

tar cvf - \
	*.messages actions ban.list bugs credits greeting help \
	help_table ideas info messages motd news policy poses typos \
	tinyworld.{mob,obj,shp,wld,zon} \
	| gzip > ../lib.tar.z
