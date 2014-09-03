>fight_prog 15
  mpsend here Craw! craw!
  mpsend here $i pecks at your head!
~
>death_prog 90
  if isinroom(quesada)
    break
  else
    mpsend here A black raven shimmers and blurs in and out of focus.
    mpsend here Quesada returns from the dead. 
    mppurge quesada-corpse
    mpload m 5409
    mpforce quesada hit $n
  endif
~
@
