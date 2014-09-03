>rand_prog 5
  mpsend here The Fortune Hunter sharpens his knife.
  say So, $r. What're you looking fer in these woods eh?
~
>greet_prog 10
if ispc($n)
  grin
  say Seen any of them fisheyed folks around?
  say I hear they got gold.
endif
~
>speech_prog fisheyed folks fish
if ispc($n)
  say You know, them Beysibs.
  say I heard they moved into town, maybe I'll drop in on them.
  ponder
endif
~
>fight_prog 5
if ispc($n)  
  say You won't need them boots where you're going $n?
  grin
  lunge $n
endif
~
>fight_prog 5
  spit $n
  grin
~
@
