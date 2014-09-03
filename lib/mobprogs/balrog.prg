>greet_prog 100
if ispc($n)
  tell $n You dare enter my home? Thou shall wither before my might!
  cast 'fireball' $n
  if rand(50)
    shout Morgoth, strengthen me against thy foes! May your will be done!
  else
    shout Dark Fire shall consume you, $n!
  endif
  cast 'fireball' $n
endif
~
>fight_prog 20
  if rand(50)
    shout Only the Wielder of the Flame of Anor can hold me back, $n!
  else
    shout I am the Flame of Udun! I will crush you, $n!
  endif
  cast 'lava storm'
  cast 'lava storm'
~
@

