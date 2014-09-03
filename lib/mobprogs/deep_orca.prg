>fight_prog 100
if ispc($n)
  shout I eat people like you for breakfast $n!
  cas 'weak' $n
  cas 'disi' $n
  cas 'frost' $n
endif
~
>act_prog p panics, and attempts to flee.
if ispc($n) 
   throw starfish $n  
   holler You think i'd let you get away!
endif
~
@
