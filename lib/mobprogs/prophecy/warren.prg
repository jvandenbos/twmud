>greet_prog 100
if ispc($n)
  say What an honor to meet you, $n.
  bow $n
  say The prophecies put great responsibility on your shoulders.
endif
~
>rand_prog 10
  say Vengeance under the Master will extinguish every adversary.
  say Terror, hopelessness, and despair will reign free.
~
>rand_prog 10
  say By winter's breath, the counted shadows shall bloom.
  say If the heir to D'Hara's vengeance counts the shadows true,
  say his umbra will darken the world. 
  say If he counts false, then his life is forfeit.
~ 
>speech_prog prophecy prophecies 
if ispc($n)
  emote quotes 'If the one named $n, fails to withstand the power of the sleepwalker..'
  emote quotes 'the boundary between our world and the underworld..'
  emote quotes 'will be broken.'
endif
~
>speech_prog responsibility what me why how
if ispc($n)
  say If you fail, not only will the Keeper have your soul..
  say he will be one step closer to reaching the world of the living.
  shiver
endif
~
>speech_prog nathan
if ispc($n)
  say Don't listen to Nathan.
  say He is just a crazy prophet, too smart for his own good.
  say Be careful with him, he tends to take prophecy into his own hands.
endif
~
>speech_prog pasha
if ispc($n)
  stare
  say Now that's a nice girl.
  say And don't you think about harming a hair on her head, $n!
endif
~
>act_prog p drops the bloody head of Pasha.
if ispc($n)
  scream
  cas 'lava'
  cas 'lava'
  cas 'lava'
endif
~
>fight_prog 90
if ispc($n)
  if rand(30)
    cas 'fireball' $n
  else
    cas 'blind' $n
    cas 'electric fire' $n
  endif
endif
~
@
