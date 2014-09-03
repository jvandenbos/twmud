>greet_prog 100
if ispc($n)
  if level($n) == 1
    say Welcome to Thieves World, $n!
    say Please make yourself at home.
    say If you have any questions, by all means ask any of the other players.
    say Oh, and to gain two free levels, type BEG right now.
    smile
  else
    if level($n) > 1
      if level($n) <= 24
        if rand(15)
          if rand(50)
            if rand(50)
              say Hello $n!
              say Another victorious excursion?
            else
              smile $n
              say Everything alright, $n?
            endif
          else
            if rand(50)
              say Enjoying your adventures in Sanctuary, $n?
            else
              say Why, it's $n!!!
              dance $n
              french $n
            endif
          endif
        endif
      endif
    endif
  endif
endif
~
>speech_prog where
if ispc($n)
  if level($n) <= 24  
    say If your having trouble finding something try "look map"
  endif
endif
~   
>speech_prog help
if ispc($n)
  if level($n) <= 24
    t $n I'm not the one to ask for help, $n.
    t $n Try asking another adventurer.
  endif
endif
~
>speech_prog zone exp experience carnival chessboard zoo deadhame shire
if ispc($n)
  if level($n) <= 24
    say $n, Go speak to the Cappen in the Grunting Boar Inn.
    say He knows all about this city and the nearby zones.
    say To find him, go: South, South, Up, Up, East.
  endif
endif
~
>speech_prog eq equipment donate donation
if ispc($n)
  if level($n) <= 20
    say The Donation Room can be a grand treasure horde!
    say I'd advise that you visit at least once, you never know what you'll find, $n.
    say To find get there, go: South, South, Up, Up, Up, East.
  endif
endif
~
>speech_prog morgue mortuary bunglebutt
if ispc($n)
  if level($n) <= 20
    say The Morgue is a function that was set up by the City of Sanctuary.
    say If you die, $n, you're corpse will be collected and returned to there.
    say You can purchase it back, for a small fee.
    say Don't worry about money, it will be placed in whole into your bank account.
    say The Morgue is: West and South from here.
  endif
endif
~
>speech_prog eq equipment donate donation
if ispc($n)
  if level($n) <= 24
    say The Donation Room can be a grand treasure horde!
    say I'd advise that you visit at least once, you never know what you'll find, $n.
    say To get there, go: South, South, Up, Up, Up, East.
  endif
endif
~
>speech_prog eq academy school learn
if ispc($n)
  if level($n) <= 10
    say The Sanctuary Academy is a great place!
    say I'd advise that you visit it at least once, $n.
    say To get there, go: South and then Up
  endif
endif
~
>speech_prog bank money gold
if ispc($n)
  if level($n) <= 24
    say The Bank is the best place to safely store your gold.
    say The people of this land are untrustworthy, even Gods may steal from you.
    say So it is always safest to not carry around large amounts if at all possible.
    say The Bank is: South, South, Up, West.
  endif
endif
~
>speech_prog auc auction auctions
if ispc($n)
  if level($n) <= 24
    say Auctions are a great way to get excellent prices on second-hand equiptment
    say Just remember to check the level and restrictions of the item before you spend you life savings on it!
    say Just type 'AUCTION HELP' to get more information on it.
  endif
endif
~
>speech_prog guild gain level pract practice learn
if ispc($n)
  if level($n) <= 24
    if class($n) & 1
      say The Mage Guild is North, then East of here, $n.
    endif
    if class($n) & 2
      say $n, the Cleric Guild is to the West.
    endif
    if class($n) & 4
      say The Warriors Guild is North then West, $n.
    endif
    if class($n) & 8
      say $n, the Thieves Guild can be located to the North then Down.
    endif
    if class($n) & 16
      say The Guild of the Paladins is Up from here, $n.
    endif
    if class($n) & 32
      say The Druid Guild is to the East, $n.
    endif
    if class($n) & 64
      say The Psionics Guild can be found North then Up, $n.
    endif
    if class($n) & 128
      say The Ranger's Guild is Down, $n.
    endif
    if class($n) & 256
      say The Guild of the ShapeShifters a longer walk.
      say Travel: South Two, West Five, and then North Once.
      say Ok, $n?
    endif
    if class($n) & 512
      say The Temple of the Monks is southeast of our location.
      say Travel: South, East Three, and then South Three.
    endif  
    if class($n) & 1024
      say The Bard Guild is South, West Thrice, then North.
      smile $n
    endif
    break  
  else
    if level($n) <= 24
      say You are too powerful the GuildMasters of Sanctuary, $n.
      say You must travel elsewhere to be able gain a level.
      say Seek out the masters who might train you.
    endif
  endif
endif
~
>rand_prog 1
if rand(50)
  if rand(50)
    ponder
    say Honor guards tend to treat those of evil alignment a little on the harsh side.
    say However, they will help those of good heart they see you in trouble.
  else
    say Many an adventurer will donate equipment to those in need.
    say In fact, there is a room for such a cause!
    smile
  endif
else
  if rand(50)
    say The Morgue is such a dreary place.
    say I hate seeing people go in there...
    sigh
  else
    say If you wanna keep your money, you'd better bank on it!
    chuckle
  endif
endif
~
>bribe_prog 1
if ispc($n)
  if rand(50)
    say For me? Thanks!
    hug $n
  else
    say A mug of Grog sounds good right now...
    ponder
  endif
endif
~
>act_prog p bows before you.
  bow $n
~
>act_prog p laughs.
if rand(80)
  ask $n hey, $n, what's so funny?
endif
~
>act_prog p screams loudly!
if rand(80)
  say Hey, chill out $n.
endif
~
>act_prog p filling the room
  cough
~
>give_prog all
  say Wow! I didn't know it was my birthday. Why thank you!
  kiss $n
  hug $n
  emote hums to himself.
~
@
