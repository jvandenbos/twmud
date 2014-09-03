>greet_prog 100
if ispc($n)
  emote greets you with his yabree.
endif
~
>fight_prog 90
if ispc($n)
  if class($n) & 1
    say Ssskin brother, I do not wisssh to fight you!
    divert $n $r
  endif
  if class($n) & 2
    say Healingss are no good here old man!
    laugh
    sing 'silence' $n
  endif
  if class($n) & 8
    stand
    say Bastardss like you, $n, sshould be kilt at birthss!
    flail
  endif
  if class($n) & 64
    say Mind readerss!
    ack
    cast 'dispel' $n
    tolerance
    cac
    if rand(50)
      flail
      mpsend char $n Your guts spill out!
      mpsend room $n $I slashes open $n's belly!
    else
      trip $n
      trip $n
      stand
      circle $n
      backstab $n
    endif
  endif
  if class($n) & 128
      say Bah, you gonna stun me $n?
      lunge $n
      cast 'dispel' $n
      cast 'web' $n
  endif
  if class($n) & 32
      emote looks around, a confused expression on his lith face.
      say You see any trees here $n?
      chuckle
      cast 'web' $n
      cast 'fireball' $n
  endif
  if class($n) & 256
      emote mutters something about shifters.
      spit $n
      cast 'creeping doom' $n
      cast 'web' $n
  endif
  if class($n) & 4
     cast 'poison gas'
     say Sso we got oursselfs a true warrior!
     disarm $n
     if rand(30)
       mpload o 6705
       hold yabree
       suggest $n remove all
     else 
       if rand(40)
         cast 'vamp' $n
         cast 'earthquake'
       endif
     endif
  endif
endif
~
>act_prog p drops A humming Yabree.
if ispc($n)
  mpsend here $I hisses:
  mpsend here Don't drop your Yabree sskin-brother.
  get yabree
  give yabree $n
  mpsend here Lissten to the ssong.
endif
~
>death_prog 100
  mpsend here $i makes a strange gurgling sound as he topples over.
  mpsend here Blood is oozing from a great gash in his belly.
  mpload o 6777
  drop guts 
~
@
