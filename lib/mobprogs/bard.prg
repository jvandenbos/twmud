>speech_prog thalos moridilinia orshingal abyss aucan githezai astral
if ispc($n)
  say I don't know of such areas, they lay outside of Sanctuary.
  say Try and find Cyrathen, he knows of such things.
  say Ask me something else!
endif
~
>greet_prog 25
if ispc($n)
  say Welcome on in!  The ale and wine flows freely here!
endif
~
>speech_prog sanctuary town
if ispc($n)
  say Whats that you'd like to know about the town of Sanctuary?
  say A fine place it is...
  say What would ya like to know?
endif
~
>speech_prog graveyard grave cemetary
if ispc($n)
  shiver
  say The graveyard?  Nasty Place!
  say Located south of the western gate, and east a bit more.
  say There is a grate which has a nasty habit of locking you inside!  Be Careful!
endif
~
>speech_prog gm guild master guildmaster
if ispc($n)
  say The guildmasters are all located near fountain square.
  say All except for the Shapeshifter Guildmaster, who is located near an odd chessboard.
endif
~
>speech_prog shop food shops
if ispc($n)
  say If its items your looking for, you want to goto the tower!
  say Its located south of the fountain, and it has your every want.
  say Laron runs the shop now that Zifnab is gone...
endif
~
>speech_prog zifnab laron
if ispc($n)
  say Laron was one of Zifnab's finest students.
  say Zifnab was reportedly slain by the vicous killer Tung Mai.
  say We all miss him greatly.
  sigh
endif
~
>speech_prog chess chessboard sisyphus
if ispc($n)
  say The chessboard is located south and west of the western gate of Sanctuary.
  say Sisyphus is the guardian there, he's got something against experienced persons.
  say I hate that.
endif
~
>speech_prog boat boats dock peer
if ispc($n)
  say Boats you want eh?  The retired captain sells boats near the docks.
  say Also boat rides can be taken to many distant lands.
  say The docks are located due south of the tower.
endif
~
>speech_prog temple
if ispc($n)
  say The Temple of Sanctuary is an awful place, filled with evil creatures of of all sorts.
  say The monsters are quite simple to defeat, but the town guard have been unable to clear the temple for some strange reason.
endif
~
>speech_prog zoo
if ispc($n)
  say The Sanctuary Zoo is located south of the fountain.
  say Reknowned for its extremely rare collection of animals, rumor has it that some very, very nasty animals are kept in the farthest portion of the Zoo.
  say I don't like the Zoo, they skimp on caretakers.
endif
~
>speech_prog park
if ispc($n)
  say The Sanctuary Town Park is located south and Western Gate of Sanctuary.
  say It houses numerous wonderful vistas and such.
  say A great place to take a break from the stress of the day.
endif
~
>rand_prog 20
if ispc($r)
  if rand(50)
    if rand(50)
      emote puts down his instrument and cracks his fingers.
    else
      emote strums a few bars of an enchanting melody.
    endif
  else
    if rand(50)
      say Can I help you find something in town, $r?
    else
      say The city of Sanctuary is a wonderous place.
      say It has captured my heart.
    endif
  endif
endif
~
@
