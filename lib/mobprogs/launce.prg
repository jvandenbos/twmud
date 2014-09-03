>greet_prog 100
if ispc($n)
  if isevil($n)
    if rand(50)
      look $n
      say I have sworn my allegiance against the likes of you, $n!
    else
      glare $n
    endif
  else
    if rand(50)
      smile $n
      tell $n Hail and well met!
    else
      say $n, you honor me!
      bow $n
    endif
  endif
endif
~
>fight_prog 50
  say A heros work is never done!!!
  bash
  stand
~
@ 

