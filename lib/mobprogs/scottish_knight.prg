>greet_prog 100
if ispc($r)
  say Death shall be quick.
  cast 'dispel magic' $n
  cast 'fireball' $n
  berserk
  cast 'vamp'
endif
~  
>act_prog p panics, and attempts to flee.
if ispc($n)
  holler Running will not save you, fiend!
endif
~
>fight_prog 100
  cast 'ice storm'
  cast 'dispel magic' $n
~
@