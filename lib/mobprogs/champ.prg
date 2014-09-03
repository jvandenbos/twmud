>greet_prog 30
if name ($n) !/ githezai
  if isimmort ($n)
    break
  endif
  if rand(50)
    shout Intruders!  You shall be SLAIN!
  else
    shout Intruders in the palace, come to me my bretheren!
  endif
  bash $n
  stand
endif
~
>rand_prog 20
if name ($r) !/ githezai
  if isimmort ($r)
    break
  endif
  if rand(50)
    shout Intruders!  You shall be SLAIN!
  else
    shout Intruders in the palace, come to me my bretheren!
  endif
  bash $r
  stand
endif
~
@

