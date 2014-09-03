>rand_prog 20
if rand(50)
  if rand(50)
    say Let me know when you're ready.
    emote begins enscribing some glyphs on a parchment.
  else
    emote flips through the pages of a large tome.
  endif
else
  if rand(50)
    say Now, where did I put that wand?
    emote begins rummaging under his desk for something.
  else
    emote utters a word and a scroll flys off a high shelf and into his hand.
  endif
endif
~
>greet_prog 20
if level($n) >= 50
  if rand(50)
    sigh
    if rand(30)
      say $n, I miss Zifnab...
    endif
  else
    if isgood($r)
      if rand(50)
        say $R!  In my store!
        say What a pleasant suprise!
      else
        say Why hello, $r.
      endif
    endif
    if isevil($r)
      if rand(50)
        emote shivers as you step into the shop.
      else
        emote takes a quick inventory of his possessions as he notices you entering his shop.
      endif
    endif
    if isneutral($r)
      if rand(50)
        say Welcome, Adventurer!
      else
        say Welcome to the Magic Shop, $r.
      endif
    endif
  endif
else
  if rand(50)
    say Hello, $r.  What can I do for ya?
  else
    mpsend As you step through the door, you see $I looking depressed.
    emote forces a smile.
    say How can I help you today, $r?
  endif
endif
~
@
