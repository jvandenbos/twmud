>greet_prog 40
if ispc($n)
  mpsend here You hear a strange hissing sound nearby.
  whisper $n $n, leave the resting ground of the Ch'tar IMMEDIATELY!
  nohassle
endif
~
>fight_prog 100
if isnpc($n)
  mpgoto 16737
endif
if ispc($n)
  growl
  say $n, Meet your DOOM!
  mptransfer $n 6024
  mpat 6024 mpsend here There is an engulfing white light!
  mpat 6024 mpforce $n look
  mpgoto 16737
endif
~
>speech_prog ch'tar ch'tarean tomb chtar ch-tar tombs
if ispc($n)
  emote hisses 'Only those bearing the dead of Ch'tar may enter the tomb.'
  emote hisses 'Put the dead of our lands on the ground as proof and you shall be allowed here to enter.'
endif
~
>act_prog p drops a mummified ch'tarean corpse.
if ispc($n)
  emote hisses 'You are not Ch'tarean, you entry into the crypts is unwanted.'
  emote hisses 'But you submitted a corpse of our citizenry, you may enter.'
  emote hisses 'I shall facilitate your entry.'
  mpload 16710
  give key $n
endif
~
>greet_prog 10
if ispc($n)
  mpsend here You feel a cold chill enter your bones.
endif
~
@
