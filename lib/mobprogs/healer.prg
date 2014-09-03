>rand_prog 50
if ispc($r)
    cast 'cure light' $r
endif
~
>rand_prog 50
if ispc($r)
    cast 'armor' $r
endif
~
>rand_prog 50
if ispc($r)
    cast 'bless' $r
endif
~
>act_prog p bleeds all over the floor.
cast 'heal' $n
~
@
