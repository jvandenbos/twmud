>greet_prog 100
if ispc($n)
  if class($n) == 1
    say The BakaBanMana must kill any magic-men they meet.
    say So says the prophecy.
    kill $n
  endif
  if class($n) == 2
    say The BakaBanMana must kill any magic-men they meet.
    say So says the prophecy.
    kill $n
  else
    bow $n
    say The BakaBanMana greets you, $n.
  endif
endif
~
>fight_prog 80
if ispc($n)
  if rand(30)
    emote moves like a wind, his blade moving like a windmill.
    if hasobj($i, blade-sword) == 1
      flail
    else
      mpload o 6725
      wield blade
      flail
    endif
  else
    circle $n
    mpsend here Where did that blade come from?
    bash $n
    stand
    trip $n
  endif
endif
~
>act_prog p gets gold coins.
if ispc($n)
  say Why would you take gold that does not belong to you, $n?
  glare $n
endif
~
>death_prog 100
  if rand(30)
    say No hard feelings, but you got lucky this time.
    emote splutters blood on your shoes.
  else
    say It was an honor dieing by your hand caharin.
    mpload o 6725
    give blade $r
  endif
~
@
