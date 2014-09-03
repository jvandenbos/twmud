>greet_prog 100
if ispc($n)
  say the clouds warned me about you $n.
  say I think your name has been written in the prophecys.
  say I was never that good with books.
  ponder
  say You should try talking to Warren. He might help you.
endif
~
>fight_prog 80
if ispc($n)
  if rand(30)
    say The clouds warned me about your temper too, $n.
    cas 'fireball' $n
  else
    say Only one of us will survive this fight; so the prophecy says.
    cas 'vamp' $n
  endif
endif
~
>act_prog p gets gold coins.
if ispc($n)
  say Don't let wealth cloud your vision, young $n.
endif
~
@