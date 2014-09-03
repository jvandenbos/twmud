>greet_prog 100
if ispc($n)
  say You will die by my blade!
  cast 'dispel magic' $n
  cast 'ice storm'
  cast 'vamp touch'
  cast 'vamp touch'
  cast 'vamp touch'
endif
~  
>fight_prog 100
  cast 'fireball' $n 
  cast 'acid rain'
  cast 'acid rain'
  cast 'dispel magic' $n
~
>death_prog 100
  shout My precious blade! Kill this fiend they call $n!
  mptransfer guardian-noble
  mpforce guardian get all
  mpforce guardian kill $n
~
@
