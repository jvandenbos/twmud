>speech_prog p heal
if ispc($n)
  if rand(80)
    cast 'heal' $n
  else
    mpsend here $i is not pleased with you disturbing his meditation.
    cast 'rupture' $n
  endif
endif
~
>speech_prog p refresh
if ispc($n)
  cast 'refresh' $n
endif
~
>speech_prog p fuck damn shit 
if ispc($n)
  if rand(80)
    cast 'fear' $n
  else
    say Go wash your filthy mouth, $n.
    cast 'water breath' $n
  endif
endif
~
>act_prog p bows before you.
if rand(80)
  cast 'sanc' $n
else
  say A pleasure to meet you $n.
  bow $n
endif
~
@