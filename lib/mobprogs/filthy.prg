>greet_prog 85
if ispc($n)
  if rand(50)
    say $n
    say What can I do ya for?
  else
    say You look like a thirsty man, $n.
    say Am I right?
    if rand(80)
      mpsend here A patron in the back chips in 'That, or he's a real ugly lady!'
      mpsend here The barroom explodes in laughter.
    endif
  endif
endif
~
>rand_prog 15
if isfight($i)
  break
else
  if rand(50)
    if rand(50)
      roll
      say Hey $r, ever think about taking a bath?  You stink!
    else
      say You've got a lot of nerve showing your face around here, $r.
      smirk
    endif
  else
    if rand(50)
      emote pours an ale and stares at it until the grit settles.
    else
      emote spit-polishes a glass and puts it back on the shelf.
    endif
  endif
endif
~
@
