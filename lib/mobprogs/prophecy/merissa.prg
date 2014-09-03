>greet_prog 100
if ispc($n)
  if rand(50)
    say Don't you ever knock, $n?
    cast 'vamp' $n
  else
    say My, my.. another hero..
    smirk
    cas 'drain' $n
    cas 'para' $n
  endif
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    cast 'chain elec'
  else
    cast 'poison gas'
    cast 'rupture' $n
  endif
endif
~
>act_prog p looks at you
if ispc($n)
  say I will not have your eyes taking what does not belong to you, $n!
  cast 'disintegrate' $n
  cast 'web' $n
endif
~
@