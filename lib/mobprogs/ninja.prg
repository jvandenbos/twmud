>entry_prog  80
if isimmort ($n)
  break
  else
  if name($n) !/ oriental
    mpsend here A ninja spots you, and ATTACKS!
    kill ($n)
  endif
endif
~
>all_greet_prog  80
if isimmort ($n)
  break
  else
  if name($n) !/ oriental
  mpsend here A ninja notices your movements, and ATTACKS!
  kill $n
  endif
endif
~
>health_prog 20
if rand(5)
 emote pulls out an odd looking whistle and BLOWS!
 mpload mob 4131
 mpsend here A ninja arrives to aid his companion!
endif
~
@
