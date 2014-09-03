>greet_prog 100
if ispc($n)
  emote charges you, fangs spread wide!
  breathe $n
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    breathe $n
  endif
  if rand(40)
    mpsend char $n $i hits you with his wing!
    mpsend room $n $n stumbles backward as $i hits $m!
  else
    mpsend char $n $i lifts you up in the air!
    mpsend room $n $i trashes $n violently, lifting $m up in the air!
    thought $n
  endif
endif
~
>fight_prog 5
  if ispc($n)
    if level($n) <= 110
      if rand(50)
        mpsend char $n $i is all over you! His fangs ripping at your flesh!
        mpsend room $n Blood splatters your face as $i rips $n open!
        mpkill $n
      endif
    endif
  endif
~
>death_prog 80
  emote topples over, clawing at the air.
  say archhgrraawrgh!
  emote clutches one claw around a leather thong tied around his neck.
~
@