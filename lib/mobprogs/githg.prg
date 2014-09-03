>fight_prog 65
if ispc($n)
  disarm
  disarm
endif
~
>rand_prog 10
if ispc($r)
  if rand(50)
    scan
  else
    mpsend char $n The $i spots you and attacks!
    mpsend room $n $i spots $n and attacks!
    kill $n
  endif
endif
~
@
