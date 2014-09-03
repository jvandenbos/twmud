>greet_prog 100
if ispc($n)
  snuggle $n
  purr
endif
~
>fight_prog 90
if ispc($n)
  if rand(50)
    flail
  else
    cast 'poison gas'
    emote bares her teeth.
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  purr
  smile $n
  mpsend room $n Was that a smile or a sneer you wonder?
endif
~
@
