>greet_prog 40
if ispc($n)
  if rand(50)
    say Well met, $n!
  else
    bow $n
  endif
endif
~
>rand_prog 5
is ispc($r)
  if rand(50)
    say The lords of death all reside on wizards wall...
    sigh
  else
    if rand(50)
      stare
    else
      ponder
    endif
  endif
endif
~
>fight_prog 100
if name($n) / stepson
  say Brother Stepson, we should not fight!
  mpforce stepson say I agree, my brother, I shall not fight thee.
  smile
  mpgo 3156
endif
~
>fight_prog 90
if isinroom(niko)
  break
else
  say Stepsons!! To Arms!!
  mptransfer niko
  mpforce niko kill $n
endif
~
>fight_prog 90
if isinroom(crit)
  break
else
  say Stepsons!! To Arms!!
  mptransfer crit
  mpforce crit kill $n
endif
~
>fight_prog 90
if isinroom(janni)
  break
else
  say Stepsons!! To Arms!!
  mptransfer janni
  mpforce janni kill $n
endif
~
>fight_prog 90
if isinroom(randal)
  break
else
  say Stepsons!! To Arms!!
  mptransfer randal
  mpforce randal kill $n
endif
~
@

