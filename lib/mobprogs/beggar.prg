>greet_prog 75
  if rand(25)
    sigh
  else
    beg $n
  endif
~
>rand_prog 25
if ispc($r)
  if rand(50)
    if rand(50)
      say $r, just a few coins for a meal. Please!
    else
      say Please $r, spare some change?
    endif
  else
    say Spare some change for the unfortunate?
    beg $r
  endif
endif
~
>bribe_prog 1
if ispc($n)
  if rand(90)
    if rand(50)
      say Bless your heart, $n.
    else
      thank $n
    endif
  else
    say Sucker!
    cackle
  endif
endif
~
@

