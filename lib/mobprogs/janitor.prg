>rand_prog 10
if ispc($r)
  if rand(50)
    if rand(50)
      if rand(50)
        emote cleans up some trash.
      else
        emote stands up straight and stares at the sky for a moment.
      endif
    else
      if rand(50)
        emote whips out a rag and begins polishing some trimwork.
      else
        emote gets down on $l knees and begins scrubbing grime off the road.
      endif
    endif
  else
    if rand(50)
      if rand(50)
        emote stops cleaning and spits tobacco.
        wipe janitor
      else
        say These long hours are killing me!
        whine
      endif
    else
      if rand(50)
        say Pick something up it won't kill ya!
        glare $r
      else
        say Mom always said I'd go nowhere in life...
        say ...but just look at me NOW!
        strut
      endif
    endif
  endif
endif
~
>bribe_prog 1
if ispc($n)
  if rand(50)
    say Ooh!  Shiny!!!
    emote stares at a coin for a good minute.
  else
    emote tosses the gold in his trash bag.
  endif
endif
~
@
