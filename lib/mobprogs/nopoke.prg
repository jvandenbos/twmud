>act_prog p pokes you in the
if isnpc($n)
  chuckle
  poke $n
else
  if level($n) <= 5 or isgood($n)
    say I would rather you didn't poke me, $n.
  else
    if level($n) > 40
      scream
      say Ya know $n, I hate being poked!!!!
      kill $n
      break
    endif
    slap $n
    say Damn, $n!!!!  Stop poking me.
  endif
endif
~
>greet_prog 100
if name($n) == static
  if rand(40)
    mpsend room $r $I sees $n, screams and runs away!
    west
    south
    say Did he see me?
  endif
else
  if rand(15)
    tell $n $N, hiya!
  endif
endif
~
>speech_prog help
if ispc($n)
  if rand(50)
    if rand(50)
      say $n, just say what you want.
    else
      say I am well versed in the ways of healing and protective magics.
      say Well, that's what Sylune the Cleric GuildMaster says.
    endif
  else
    if rand(50)
      say I'd like to help you, $n.  What do you need?
    else
      say Cure Light Wounds will restore your life points.
      say Refresh is a spell used to gain back stamina.
      say The Armor spell is useful for avoiding enemy attacks!
    endif
  endif
endif
~
>speech_prog cure heal hp hitpoints hitpoint
if ispc($n)
  if level($n) <= 50
    if rand(75)
      cast 'cure light' $n
    else
      cast 'cure serious' $n
    endif
  else
    if rand(50)
      say I can't possibly help someone of your stature, $n.
    else
      say $n, certainly YOU don't need help from me?!
    endif
  endif
endif
~
>speech_prog movement move mv refresh ref
if ispc($n)
  if level($n) <= 50
    cast 'refresh' $n
  else
    if rand(50)
      say I can't possibly help someone of your stature, $n.
    else
      say $n, certainly YOU don't need help from me?!
    endif
  endif
endif
~
>speech_prog armor ac shield protection protect
if ispc($n)
  if level($n) <= 50
    cast 'armor' $n
  else
    if rand(50)
      say I can't possibly help someone of your stature, $n.
    else
      say $n, certainly YOU don't need help from me?!
    endif
  endif
endif
~
>speech_prog static
if ispc($n)
  if rand(50)
    say Man, I hate that guy!
    say What moron would name himself 'Static'?!
    smirk
  else
    roll
    say I'm better than him.  No question.
  endif
endif
~
>rand_prog 25
if ispc($r)
  if rand(50)
    if rand(50)
      look $r
    else
      bow $r
    endif
  else
    if rand(50)
      say Hello, $r
    else
      say Hello, $r.
      say I am $I.
      say If you need assistance, just say the magic word.
    endif
  endif
endif
~
>bribe_prog 1
if ispc($n)
  if rand(50)
    say This'll help to pay for my training!
    say Thanks, $n.
  else
    say Hey, thanks.
  endif
endif
~
@

