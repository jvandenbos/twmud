#!/bin/csh -f

grep '^#' $1 > $$.raw~
sort +.1n $$.raw~ -o $$.sorted~
diff $$.raw~ $$.sorted~
rm -f $$.raw~ $$.sorted~
