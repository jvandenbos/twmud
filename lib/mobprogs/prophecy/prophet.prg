>greet_prog 100
if ispc($n)
  say Who be there?
  raise
endif
~
>speech_prog blind
if ispc($n)
  say I might be blind, but I can see better than most.
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    say Whomever you be, I pity you.
    cas 'acid rain'
  else
    say Damn you, attacking a chained, blind man.
    cas 'ice storm'
  endif
endif
~
>death_prog 80
  drop eye
  emote screams loudly, clutching at a vacant eyesocket.
~
@
