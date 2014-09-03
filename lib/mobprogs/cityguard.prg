>greet_prog 15
if ispc($n)
  mpsend char $n $I looks you over to see if you're a troublemaker.
  mpsend room $n $I eyes $n for a moment.
endif
~
>bribe_prog 1
if ispc($n)
  if rand(50)
    if rand(50)
      say Very well, I didn't know we got kickbacks.
      chuckle
    else
      say Money for more doughnuts!
      hug $n
      say Enos! Here I come!!!
    endif
  else
    if IsEvil($n)
      if rand(50)
        say Is this blood money?
      else
        if level($n) >= 20
          say A bribe?!  I will never accept this!!!
          kill $n
        else
          say I'll return this to my superior.
          emote holds the gold out in his hand as if it were diseased.
        endif
      endif
    else
      if rand(50)
        thank $n
      else
        say Your hospitality is most welcome, $n
      endif
    endif
  endif
endif
~
>act_prog p slaps you
if ispc($n)
  if rand(75) and level($n) >= 20
    say That's it! I've had enough of you, $n!
    if rand(50)
      disarm $n
      kill $n
    else
      kill $n
      bash
      stand
    endif
  else
    growl
  endif
endif
~
>act_prog p pokes you
if ispc($n)
  if rand(30) and level($n) >= 20
    say That's it! I've had enough of you, $n!
    if rand(50)
      disarm $n
      kill $n
    else
      kill $n
      bash
      stand
    endif
  else
    growl
  endif
endif
~
>act_prog p bonks you
if ispc($n)
  if rand(50) and level($n) >= 20
    say That's it! I've had enough of you, $n!
    if rand(50)
      disarm $n
      kill $n
    else
      kill $n
      bash
      stand
    endif
  else
    growl
  endif
endif
~
>act_prog p punches you
if ispc($n)
  if rand(40) and level($n) >= 20
    say That's it! I've had enough of you, $n!
    if rand(75)
      disarm $n
      kill $n
    else
      kill $n
      bash
      stand
    endif
  else
    growl
  endif
endif
~
>rand_prog 5
if isgood($r) and ispc($r)
  look $r
  if level($r) <= 20
    if rand(50)
      if rand(50)
        say Greetings, young $r.
      else
        bow $r
      endif
    else
      if rand(50)
        say You'll make it big one day $r, I can tell.
        say Never give up hope!
      else
        say Finding your way around Sanctuary alright, $r?
      endif
    endif
  else
    if level($r) <= 60
      if rand(50)
        if rand(50)
          say The fates are smiling on you, $r... I can sense it.
          smile $r
        else
          say Greetings, $r!
        endif
      else
        if rand(50)
          say It's always a pleasure to see you in these parts, $r.
        else
          bow $r
        endif
      endif
    else
      if rand(50)
        if rand(50)
          say Hail $r!  The hero has returned!
        else
          say Greetings, $R!
        endif
      else
        if rand(50)
          say Welcome back, $r.
          bow $r
        else
          say You're welcome here anytime, $r.
          say Make yourself at home!
          bow $r
        endif
      endif
    endif
  endif
  break
endif
if isevil($r) and ispc($r)
  glare $r
  if level($r) <= 20
    if rand(50)
      if rand(50)
        say You've started down a bad road, $r.
        poke $r
      else
        say $r, don't cause any trouble around here or you'll regret it.
      endif
    else
      if rand(50)
        say I'll be keeping my eye on you, $r.
        say Stay outta trouble.
      else
        say $r, it's not too late for you to change your evil ways.
      endif
    endif
  else
    if level($r) <= 60
      if rand(50)
        if rand(50)
          say You know you're not welcome here, $r.
          say Make your stay a short one.
        else
          say Keep it clean, punk!
          point $r
        endif
      else
        if rand(50)
          say Don't do anything funny, or you'll regret it.
          accuse $r
        else
          say There is good in you, $r.  I can sense it.
        endif
      endif
    else
      if rand(50)
        if rand(50)
          say Curse the gods! What dragged your sorry ass back here?
          say You're not welcome here, $r.
        else
          mpsend char $r $I grumbles a curse as he spots you.
          mpsend room $r $I grumbles a curse as he spots $r.
        endif
      else
        if rand(50)
          spit $r
          say Vermin!  Leave before I find an excuse to skin ya!
        else
          say 
        endif
      endif
    endif
  endif
  break
endif
if isneutral($r) and ispc($r)
  look $r
  if level($r) <= 20
    if rand(50)
      if rand(50)
        say Keep your act clean, $r.
      else
        say Watch yourself $r, this town's not entirely safe...
      endif
    else
      if rand(50)
        say Welcome to Sanctuary, $r.
      else
        say Be wary when adventuring, $r.
        say Outside of this town lies dangerous country.
      endif
    endif
  else
    if level($r) <=60
      if rand(50)
        if rand(50)
          mpsend char $r $I looks you over to see if you're a troublemaker.
          mpsend room $r $I eyes $r for a moment.
        else
          say Beware those with mischievous souls, $r.
        endif
      else
        if rand(50)
          say Welcome back to Sanctuary, $r.
          smile $r
        else
          say Enjoy your stay in Sanctuary, $r.
          smile $r
        endif
      endif
    else
      if rand(50)
        if rand(50)
          mpsend char $r $I gives you a quick nod of approval.
          mpsend room $r $I gives $r a quick nod.
        else
          say Adventurer $r, welcome back to Sanctuary!
        endif
      else
        if rand(50)
          say Greetings, $r.
        else
          smile $r
        endif
      endif
    endif
  endif
  break
endif
~
@
