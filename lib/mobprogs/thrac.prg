>all_greet_prog 50
if ispc($n)
  growl
  say Welcome $n, to your worst nightmare!
  grin
  say I hope you enjoy your stay.
  say I know I will!
  emote roars as two beholders suddenly appear!
  mpload m 349
  mpforce beholder follow thracdanilth
  mpforce beholder kill $n
  mpload m 349
  mpforce beholder follow thracdanilth
  mpforce beholder kill $n
  emote roars as two lensman march into the room!
  mpload m 356
  mpforce lensman follow thracdanilth
  mpforce lensman kill $n
  mpload m 356
  mpforce lensman follow thracdanilth
  mpforce lensman kill $n
  shout Go my minions, show $n a new meaning to the word AGONY!
  assist beholder
endif
~
>fight_prog 50
if ispc($n)
  shout Kill $n!  Destroy all who oppose my might!
  cast 'fireball' $n
  cast 'fireball' $n
  shout Show no mercy!
  cast 'vamp' $n
  cast 'vamp' $n
  cast 'vamp' $n
endif
~
@

