>rand_prog 80
if isfight($i)
  break
else
  mpsend here The tentacle glides slowly back under the surface...
  mppurge tentacle
endif 
~
>rand_prog 40
  mpsend here FLEE, you FOOLS!!
~
>rand_prog 60
  mpsend here RUN!  Run, while you still can!
~
>rand_prog 20
if ispc($n)
  kill $n
endif
~
@

