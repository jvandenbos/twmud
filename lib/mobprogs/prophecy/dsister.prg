>fight_prog 90
if ispc($n)
  if rand(50)
    cast 'electric fire' $n
  else
    cast 'disintegrate' $n
  endif
endif
~
>fight_prog 10
if ispc($n)
  if rand(50)
    cast 'poison' $n
  else
    cast 'para' $n
  endif
endif
~
@
