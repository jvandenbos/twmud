>greet_prog 50
if ispc($n)
  if rand(50)
    if rand(50)
      yodel $n
      say That yodel brought to you by Mo Enterprises...
    else
      emote hisses 'The Ring is nearby...'
      look $n 
    endif
  else
    if rand(50)
      glare $n
    else
      emote hisses 'We must find the Ring.'
      scan
    endif
  endif
endif
~
@

