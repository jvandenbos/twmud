>fight_prog 40
if rand(50)
  if rand(50)
    startle
    swear SSSShhhiitt!
  else
    emote hisses 'You'll never suc-ssseed, $n!'
    trip $n
    stand
  endif
else
  if rand(50)
    shout The temple has been inflitrated!  Ssstop them!!!
  else
    shout Sentinelsss!  We must stop the one they call $n!
    trip $n
    stand
  endif
endif
~
>rand_prog 20
if ispc($r)
  if rand(50)
    emote hisses.
  else
    mpsend char $r Scans the area and sees you!
    mpsend room $r Scans the area and sees $r!
    kill $r
  endif
else
  emote lowers a spear and hisses toward $r.
endif
~
>greet_prog 30
if name ($n) !/ lizardman
  if ispc($n)
    mpsend char $n Your blood runs cold as $i hisses and charges at you!
    mpsend room $n Suddenly, $n is charged by $i!
    trip $n
  endif
else
  emote lowers a spear and hisses toward $n.
endif
~
@

