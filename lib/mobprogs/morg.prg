>greet_prog 80
if ispc($n)
  if level($n) > 20
    if rand(50)
      hug $n
      say $n, you never visit anymore!
    else
      say Long time no see, $n.
    endif
  else
    if rand(50)
      say Welcome, $n.
      say Did you lose something?
    else
      say Why hello, $n.
      smile $n
    endif
  endif
endif
~
>rand_prog 30
if ispc($r)
  if rand(50)
    if rand(50)
      say If you've died recently, type "List" to see if I've found you.
      say If I have what you want, type "Buy $r".
    else
      say Everybody needs a body.
    endif
  else
    if rand(50)
      say Have you lost something important recently, $r?
    else
      say Bunglebutt only retrieves corpses of players up to level 20.
      say After that, you'll have to get it on your own.
    endif
  endif
endif
~
@

