>greet_prog 100
if ispc($n)
  say I have read about you in the prophecys, $n.
  say I guess I should warn you.
  ponder
endif
~
>speech_prog warn
if ispc($n)
  say It was just some silly thing about you and death.
  say It might not even be a true prophecy.
endif
~
>speech_prog prophecy
if ispc($n)
  say When evil haunts the night, the hot-tempered one
  say will see death in the land of the time palace.
  shrug
endif
~
>speech_prog time palace
if ispc($n)
  say This palace was under a time-spell a long time ago.
  say The sisters and their captives aged very slowly,
  say compared to the outside world.
endif
~
>speech_prog death
if ispc($n)
  say Quite simple really.
  say Death as in dead, morte, six feet under.
  say It might not be you who dies. But someone will.
endif
~
>rand_prog 5
  say Have I told you that this plan of yours is madness??
~
>rand_prog 5
  say I'd like to live to see a thousand you know.
  say You're going to get us both killed.
~
>fight_prog 20
if ispc($n)
  if rand(30)
    say You got a sword for me, $n.
    thought $n
    say I would look dashing with a sword at my hip.
    cackle
  else
    say I guess the prophecys are right about your temper, $n.
    mpsend char $n $i backhands you across your face!
    mpsend room $n $i backhands $n across the face!
    chuckle
    ego $n
    awe $n
  endif
endif
~
@
