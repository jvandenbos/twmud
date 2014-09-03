>greet_prog 100
if ispc($n)
  emote blushes slightly as you enter the room.
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    snuggle $n
    suggest $n kiss pasha
    suggest $n give bag pasha
  else
    giggle
    cast 'poison' $n
    sing 'silence' $n
    kiss $n
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  blush
  say oh $n!
  french $n
  awe $n
endif
~
@
