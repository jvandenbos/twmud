>act_prog p bows before you.
if rand(70)
  cast 'ray'
  cast 'str'
  cast 'str'
  wield rival
  shout Ah $n, you have honor! 
endif
~
>fight_prog 100
  cast 'lava'
  cast 'lava'
  cast 'dispel magic' $n
~
>rand_prog 2 
if rand(50)
  if rand(5)
    mpload m 20114
    hit goblin
    Holler You are boring me stupid mortals.
    Holler Even the goblins are brave enough to come and play!
  else
    scan
    sigh
    Shout Come kill me if you dare
  endif
else
  if rand(5)
    Holler Nobody wishes to fight me?
    Holler You are all too weak still.. *sigh*
  else
    Holler Bring me my tea!
    Holler Damn, sometimes being so good is boring!
    Holler No one will come and fight me anymore.
  endif
endif
~
>death_prog 100
  shout Guardian! Avenge my death! Kill $n!
  mptransfer alkian-guardian-demonspawn
  mpforce guardian kill $n
~
@

