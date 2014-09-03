>greet_prog 100
if ispc($n)
  ' How can I help you, $n?
  smile
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    cast 'acid rain'
    cast 'heal' self
  else
    cast 'heal'
    cast 'ice storm'
    cast 'heal'
    cast 'vamp' $n
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  smile
  say Sweet you always were, $n.
  say I remember your first steps.
  dream
endif
~
>death_prog 70
  mpsend here $i draws a spell in the air, giving her life to the magic.
  cas 'lava'
~
@
