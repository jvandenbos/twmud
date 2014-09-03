>greet_prog 100
if ispc($n)
  say What! How dare you enter this SACRED LAND!
  shout You shall die at my hand, $n!
  cast 'dispel magic' $n
  cast 'fireball' $n
  mpforce imp assist master
  mpforce 2.imp assist master
endif
~
>fight_prog 100
  cast 'dispel_magic' $n
  cast 'vamp'
  cast 'fireball'
  cast 'lava'
~
>death_prog 100
  shout So, you have killed me this time!
  shout Fill in the gaps my impish friends!
  mptransfer spark-imp
  mpforce imp cast 'call lightning'
  mptransfer energic-imp
  mpforce imp assist 2.imp
~
@

