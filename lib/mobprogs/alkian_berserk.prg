>greet_prog 100
if ispc($n)
  cackle
  kill $n
endif
~
>fight_prog 100
  cast 'dispel magic' $n
  cast 'vamp'
  berserk
~
>death_prog 100
  shout NO! You fools will die for this!
  mptransfer alkian-warrior
  mpforce warrior kill $n
  mptransfer alkian-warrior
~
@

