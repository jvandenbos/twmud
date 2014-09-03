>rand_prog 50
if rand(50)
  beg
else
  sigh
endif
~
>rand_prog 50
if ispc($n)
  ask $n Can you spare some money?
  beg $n
endif
~
>greet_prog 100
if ispc($n)
  stand
  wield alkian-beggar
  backstab $n
  remove all
  laugh $n
endif
~
@

