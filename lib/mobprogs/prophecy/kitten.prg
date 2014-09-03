>greet_prog 100
if ispc($n)
  if name($n) == dar
    mpsend here The little kitten utters the words 'meeeoww'.
    mpsend here The little kitten is overjoyed at seeing her dadda.
    lick $n
    fol $n
    purr
  endif
  if name($n) == quilan
    mpsend here The kitten is overjoyed at the sight of Quilan and jumps 
into his lap.
    lick $n
    fol $n
    purr
  endif
  if name($n) == treyth
    stare $n
    fol $n
    purr
  endif
  if name($n) == atoyo
    mpload o 3001
    mpload o 3001
    give beer atoyo
    drink beer
    burp
    fol $n
    purr
  endif
  if name($n) == ruben
    mpsend here The little kitten thinks Old man Ruben is just -so- sweet.
    lick ruben
    fol $n
    purr
  endif
  if name($n) == flower
    mpsend here The little kitten jumps into Flower's lap.
    purr
  endif
  if name($n) == axe
    mpsend here The little kitten don't think Axe is all that scary.
    purr
    snuggle $n
  else
    bite $n
    purr
  endif
endif
~
>fight_prog 90
if rand(85)
  if rand(50)
    form claws
  else
    emote bares her teeth.
    melt
    return
    hit $n
  endif
endif
~
>act_prog p kisses you.
if ispc($n)
  purr
  smile $n
  mpsend room $n Was that a smile or a sneer you wonder?
endif
~
@
