>greet_prog 50
if ispc($n)
  giggle
  snuggle $n
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    cast 'acid blast' $n
    cast 'heal' self
  else
    cast 'heal'
    cast 'frost cloud' $n
    cast 'heal'
  endif
endif
~
@