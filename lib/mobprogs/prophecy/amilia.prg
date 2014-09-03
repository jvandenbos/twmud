>greet_prog 100
if ispc($n)
  emote blushes slightly as you enter the room.
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    snuggle $n
    suggest $n rem all
    suggest $n drop all
  endif
  if rand(10)
    cast 'fireball' $n
    laugh
  endif
  if rand(40)
    cast 'electric fire' $n
  else
    wiggle
    grin
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  blush
  say Why $n, I never knew!
  french $n
  follow $n
  mpsend here Something about $I makes your blood flow faster.
  adren $n
endif
~
>death_prog 100
  snif
  say You b-b-bastard..
  emote coughs blood all over you as she topples over in your arms.
~
@