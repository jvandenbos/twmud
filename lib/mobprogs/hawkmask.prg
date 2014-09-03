>greet_prog 15
if ispc($n)
  if rand(50)
    emote eyes you cautiously...
  else
    mpsend here As you enter the room, $i turns to face you.
  endif
endif
~
>rand_prog 10
if ispc($r)
  if rand(50)
    if rand(50)
      emote readjusts his mask.
    else
      glare $r
    endif
  else
    if rand(50)
      say Jubal owns this town.
      say He owns you too!
      poke $r
    else
      say The mighty Jubal is the one who is really in charge of this city.
      cackle
    endif
  endif
endif
~
>fight_prog 10
  trip $n
~
>fight_prog 10
  bash
~
@

