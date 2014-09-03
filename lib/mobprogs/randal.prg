>greet_prog 10
if ispc($n)
  if rand(50)
    smile $n
    ask $n Have you seen any other Stepsons?
  else
    say My brethren!  Where have you gone?!
  endif
endif
~
>rand_prog 20
if ispc($n)
  if rand(50)
    if rand(50)
      say I must return to Wizards Wall, soon!
      shiver
    else
      look $n
    endif
  else
    if rand(50)
      say The Dark One, Mandraib, must be silenced, forever!
      sneer
    else
      say I fear what must be done soon...
    endif
  endif
endif
~ 
>speech_prog stepsons stepson
if ispc($n)
  mpsend here Randal says:
  mpsend here The Stepson are the elite mercenary clan of Ranke, they are sponsored
  mpsend here by more than a few gods. Their leader is said to be the actual son
  mpsend here of Stormbringer the god of war, cast out as a mortal to punish him
  mpsend here for his lack of obedience. This group consists of matched pairs,
  mpsend here bonded and sworn as partners for life, their loyalty to each other 
  mpsend here is only surpassed by thier allegience to there leader Tempus Thales.
  mpsend here We are the only guild to ascend the ranges of Bausikil and lay seige
  mpsend here to Wizards Wall, yet lived and remain strong enough to tell the tales 
  mpsend here of those  many great adventures!
endif
~
>speech_prog wizards wall
if ispc($n)
  shiver
  mpsend here Randal says:
  mpsend here Wizards Wall, is an ancient place of great power. A training ground
  mpsend here for the greatest majiks the world has ever known.  It lies high atop
  mpsend here the Bausikil mountan range far north of Ranke, and is the home of great 
  mpsend here hoards of evil clans, ruled by Mandraib the Dark One.  The lands about
  mpsend here his domain are fouled and spoiled by countless eons of death and dark
  mpsend here majiks, a trip to his domain should be well planned indeed.
endif
~
>speech_prog mandraib dark one
if ispc($n)
  mpsend here Randal says:
  mpsend here Mandraib the Dark One, is the lord of evil, son of the god of pain and death. 
  sigh
  mpsend here He is the keeper of the greatest source of magic in all the universe,
  mpsend here forged by the gods before the creation, it is the dark half, the source
  mpsend here of evil magic, the well spring from which all black magic flows.
  mpsend here Mandraib is obsessed with its power and protects it relentlessly, and will
  mpsend here sacrifice all to keep it safe within his putrid empire.
endif
~
>fight_prog 15
  say This is gonna hurt, $n!!!
  cast 'implode' $n
  cast 'fire wind' $n
~
>bribe_prog 1
if ispc($n)
  if rand(50)
    shout The Stepsons are not in need of donations!!!
    grumble
    emote pockets the money.
  else
    shrug
  endif
endif
~
@
