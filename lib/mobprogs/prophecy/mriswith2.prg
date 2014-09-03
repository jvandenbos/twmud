>greet_prog 100
if ispc($n)
  emote greets you with his yabree.
endif
~
>fight_prog 75
if ispc($n)
  if rand(40)
    cast 'vamp' $n
    cast 'earthquake'
  else
    bind $n
    constrict $n
    bind $n
    melt
    return
    hit $n
    melt
    return
    get yabree
    wield yabree
    trip $n
    stand
  endif
endif
~
>death_prog 100
  mpsend here With a crying sound, $i falls to the ground dead.
  mpsend here As life is extinguished in $l eyes,
  mpsend here $l yabree slips from $l grip.
  mpload o 6705
  drop yabree
~
@
