>greet_prog 100
  if ispc($n)
   emote sees your insignificant form and laughs out loud.
   tell $n I will dine on your soul, insect!
   cast 'call lightning' $n
   cast 'web' $n
   cast 'web' $n
  endif
~
>act_prog p panics, and attempts to flee.
  if rand(70)
   rumor Fools rush in, and cowards flee!
  endif
~
>act_prog p is dead! R.I.P.
  rumor *lipsmack* Another tasty morsel! *fingerlick*
~
>fight_prog 100
   cast 'call lightning'
   cast 'call lightning'
   cast 'web'
~
@
