>greet_prog 100
if ispc($n)
  if class($n) == 1
    shout Your scalp is mine $n!
    say I'll patch it onto my cloak.
    say right next to the whore I kilt yesterday.
    laugh
    kill $n
  endif
  if class($n) == 2
    shout Your scalp is mine $n!
    kill $n
  else
    bow $n
    say So are you with us or against us, $n?
  endif
endif
~
>fight_prog 50
if ispc($n)
  bash $n
  stand
endif
~
>act_prog p is dead! R.I.P.
  rumor Muahaha! Hurt did it, $n?
  if hasobj($i, cloak-scalps) == 1
    mpsend here Stein adds another scalp to his cloak.
  else
    mpload o 6745
    mpsend here Stein adds another scalp to his cloak.
  endif
  cac
~
@
