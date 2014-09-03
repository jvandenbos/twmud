>greet_prog 100
if ispc($r)
  hypno $r
  order followers drop all
  order followers remove all
  order followers drop all
  get all
  mpsend The Grand Master Waves you goodbye!
  order follwers eat berry
  shout Wow! What beautiful equipment you HAD!	
endif
~
>act_prog p panics, and attempts to flee.
if ispc($n)
  shout Come back, don't you think I am pretty?
endif
~
>fight_prog 100
  cast 'dispel magic' $n
  berserk
  cast 'vamp'
  cast 'vamp'
  cast 'vamp'
  cast 'lava'
~
@

