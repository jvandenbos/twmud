>greet_prog 60
if isgood($n)
  if rand(30)
    mpsend char $n $I wags $l tail at you.
    mpsend room $n $I wags $l tail at $n.
  endif
else
  growl $n
endif
~
@
