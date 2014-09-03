>greet_prog 100
if ispc($n)
  if rand(50)
    say Do you know my name, $n?
    say It's Ischade.
  else
    say Don't be nervous, $n.
    smirk
    You're not my type.
  endif
endif
~
>fight_prog 90
if ispc($n)
  if rand(40)
    cast 'dispel' $n
    cast 'poison gas'
  else
    cast 'vamp' $n
    cast 'energy drain' $n
  endif
endif
~
>act_prog p looks at you.
if ispc($n)
  mpsend char $n Looking into $i's eyes, you suddenly realize you've made a huge mistake!
  mpsend room $n $n is completely locked in $i's gaze!
  if level($n) <= 50
    hypno $n
    order $n remove all
    order $n hit ischade
  else
    thought $n
    cast 'vamp' $n
  endif
endif
~
>speech_prog fuck damn shit ass bitch
if ispc($n)
  say Don't trade curses with me, $n. 
  say You would not profit in the exchange.
endif
~
>act_prog p swears
if ispc($n)
  if rand(10)
    shout Don't trade curses with me, $n!
    shout You would not profit in the exchange.
  endif
endif
~
>weather_prog midnight
  sigh
  mpload m 5447
  say You have until dawn my dear Sjekso.
~
>act_prog p panics, and attempts to flee.
if ispc($n)
  holler Oh no you don't!
  mptransfer $n
endif
~
@
