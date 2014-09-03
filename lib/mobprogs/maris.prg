>all_greet_prog 15 
if isimmort($n)
  break
else
  if ispc($n)
    if rand(50)
        holler Intruders!  None shall enter the palace!
      else
        holler Help me fellow soldiers, we shall destroy the invaders!
    endif
    order followers kill $n
    kill $n
  endif
endif
~
@
