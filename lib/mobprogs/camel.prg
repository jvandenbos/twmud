>fight_prog 80
  kick $n
~
>fight_prog 20
  kick $n
~
>health_prog 30
  cry
~
>rand_prog 10
if ispc($r)
  if rand(50)
    if rand(50)
      emote lowers it's head to the ground.
    else
      emote turns it's head away from you.
    endif
  else
    spit
  endif
endif
~
@

