>greet_prog 100
if ispc($n)
  mpsend here A gust of foulsmelling wind whispers:
  mpsend here 'Leave this place, $n.'
endif
~
>fight_prog 90
if ispc($n)
  if rand(40)
    emote screams 'Your soul is forfeited now $n! To the Keeper you go!
    cas 'dispel' $n
    cas 'rupture' $n
  else
    cas 'wither' $n
    cas 'vamp' $n
  endif
endif
~
@