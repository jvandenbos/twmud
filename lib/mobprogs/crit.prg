>fight_prog 100
if name($n) / janni
  say Brother Stepson, we should not fight!
  mpforce stepson say I agree, my brother, I shall not fight thee.
  smile
  mpgo 3156
endif
~
>fight_prog 90
if isinroom(straton)
  break
else
  say Stepsons!! To Arms!!
  mptransfer straton
  mpforce straton kill $n
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

