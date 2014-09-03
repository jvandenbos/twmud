>greet_prog 100
if ispc($n)
  if rand(90)
    say Oh my, don't stand there drooling $n. It doesn't become you.
    cas 'electric fire' $n
  endif
endif
~
>fight_prog 90
if ispc($n)
  if rand(60)
    cas 'chain elec'
  else
    cas 'electric fire' $n
    cas 'energy drain' $n
  endif
endif
~
>act_prog p looks at you.
if ispc($n)
  say I will not have your eyes taking what does not belong to you, $n!
  cast 'disintegrate' $n
  cast 'web' $n
endif
~
>death_prog 80
  if hasobj($i, ulicia-ring) == 1
    mpsend here Ulicia clutches a hand to her throat; blood oozing between her fingers.
    say You beat me this time.. *coughs blood*
    say but you leave emptyhanded..
    mpsend here Ulicia draws one last spell in the air, giving herself to the magic.
    mpat demmin-nass give ring demmin
    cas 'earthquake'
    mppurge self
  else
    mpsend here As death claims Ulicia, a shriek of terror fills the night.
    mpsend here The terror of a soul sworn to the Keeper.
  endif
~
@
