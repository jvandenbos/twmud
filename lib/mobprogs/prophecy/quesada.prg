>greet_prog 100
if ispc($n)
  curse
  say Oops, sorry about that $n. How can I help you?
  blush
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    emote starts waving her hands in the air.
    cast 'frost cloud' $n
    cast 'frost cloud' $n
  else
    mpsend here $i's black hairlock suddenly glows bright blue!
    suggest $n cast 'dispel'
    suggest $n berserk
    cast 'vamp' $n
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  blush
  say You're so sweet $n.
  say But it wouldn't be fair of me to take advantage of your innocense.
  hug $n
endif
~
>death_prog 70
  mpsend here $i makes a wild gesture with her hands.
  mpsend here Bright light shoots from her fingertips, enlightning the air.
  mpsend here As she falls to the ground, a black raven materialize from the light.
  mpload m 5437
  mpforce raven kill $n
~
@
