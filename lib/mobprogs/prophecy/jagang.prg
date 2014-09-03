>greet_prog 100
if ispc($n)
  if rand(50)
    say Too late to run now, $n.
    snicker
    drain $n
  else
    grin
    tol
    brainstorm $n
  endif
endif
~
>fight_prog 30
if ispc($n)
  if isinroom(merissa)
    break
  else
    if rand(50)
      mpsend here $i has summoned help from the underworld!
      mpload m 5431
      mpforce merissa fol jagang
      mpforce merissa kill $n
    else
      mpsend here $i has summoned help from the underworld!
      mpload m 5427
      mpforce gozgor fol jagang
      mpforce gozgor kill $n
    endif
  endif
endif
~
>fight_prog 50
if ispc($n)
  if rand(50)
    thought $n
    melt
    return
    form blades
    kill $n
  else
    phant
    order ghost kill $n
    melt
    return
    form claws
    kill $n
  endif
endif
~
>act_prog p leaves south.
if ispc($n)
  south
  blast $n
endif
~
>act_prog p leaves north.
if ispc($n)
  north
  blast $n
endif
~
>act_prog p leaves east.
if ispc($n)
  east
  blast $n
endif
~
>act_prog p leaves west.
if ispc($n)
  west
  blast $n
endif
~
>act_prog p panics, and attempts to flee.
if ispc($n)
  mpat $n thought $n
  mpat $n whisper $n So you think escaping me is -that- easy?
  mpat $n chuckle
endif
~
@
