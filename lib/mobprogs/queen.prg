>all_greet_prog 100
if name($n) / eagle
  mpforce eagle mpgo 3385
endif
~
>fight_prog 50
if isinroom(eagle)
  break
else
  mpsend here The Queen sounds the royal alarm!
  mptransfer eagle king
  mpload m 3288
  mpforce royal kill $n
  mpforce eagle kill $n
endif
~
>fight_prog 100
if name($n) / eagle
  say Oh great Eagle King, we should not fight!
  mpforce eagle say I agree, my Queen, I shall not fight thee.
  mpsend here The Eagle King makes strange motions about with his hands!
  mpforce eagle mpgo 3385
  mpgo 3384
  smile
endif
~
@

