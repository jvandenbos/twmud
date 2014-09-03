>rand_prog 100
if isinroom(royal)
  if isfight($i)
    break
  else
    say I am no longer in jeapordy my loyal protectors, you may go.
    mpsend here The Royal Falconian Guards having performed their duty leave...
    mppurge royal
  endif
endif 
~
>greet_prog 50
if ispc($n)
  smile
  say Ah, I guess you'll be staying in our kingdom for awhile, $n?
  emote smiles as he waits for a reply.
endif
~
>fight_prog 15
if isinroom(royal)
  break
else
  mpsend here The King shouts 'Guards!!  Usurpers are set upon thy monarch! Help me!'
  mpload m 3288
  mpforce royal kill $n
  mpload m 3288
  mpforce royal kill $n
  mpload m 3288
  mpforce royal kill $n
  mpsend here The Royal Falconian guards have arrived!!!!
  mpsend here They shout in unison 'To arms!! Save the King!!!'
endif
~
@
