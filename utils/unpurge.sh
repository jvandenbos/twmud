#!/bin/csh -fx

cd $LIB

gzip -dc purged-$1.tar.gz | tar xvf - players.d/back/$2

set dir = `echo $2 | awk '{ print substr($1,1,1) }'`

mv players.d/back/$2 players.d/$dir/$2
