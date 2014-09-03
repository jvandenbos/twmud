>greet_prog 100
if ispc($n)
  say $n, you have made it far.  But you will die here!
  cast 'poison gas'
  cast 'poison gas'
  cast 'poison gas'
endif
~
>act_prog p panics, and attempts to flee.
if isinroom(healer)
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
  break
else
  mptransfer m alkian-healer
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
  mpforce healer cast 'heal' master
endif
~
@

