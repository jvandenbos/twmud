>greet_prog 100
if ispc($n)
  say Do you want anything friend?
endif
~
>rand_prog 20
if ispc($r)
  say I really could use a better hammer. This one sucks!
  beg $r
endif
~
>rand_prog 20
  say One day I'm gonna find the one who destroyed my shop..
  say and crack his skull.
~
>rand_prog 20
if ispc($r)
  say You need some help $r?
endif
~
>rand_prog 20
if ispc($r)
  say Just gimme your money $r. I'll let you live.
  cackle
endif
~
>bribe_prog 99
if ispc($n)
  if rand(50)
    give rose $n
  else
    say ahaha, why would I need your money $n?
    give 100 coins $n
    give orb $n
    ruffle $n
  endif
endif
~
>fight_prog 20
if ispc($n)
  chuckle
  say Dumbass, you can't kill me. I'm already dead.
  laugh
endif
~
@