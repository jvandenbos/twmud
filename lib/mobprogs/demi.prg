>greet_prog 90
if ispc($n)
  if ischarmed($n)
    say Ooo! A visitor...
  else
    hypno $r
    order followers drop all
    order followers kill $n
  endif
endif
~
>greet_prog 50
if ispc($r)
  hypno $r
  order followers rem all
  order followers drop all
  get all.bag
  get all.quiver
  get all.hole
  get all.pack
endif
~
>rand_prog 75
if ispc($r)
  if ischarmed($r)
    smile
  else
    order followers kill $r
    kill $r
  endif
endif
~
>rand_prog 7
  hypno $r
~
>act_prog p Blood splatters into the room.
  shout Blood spattering into my room, how charming!
~
>act_prog p What was that...a death scream?
  shout What sweet music, a death scream certainly is!
~
@

