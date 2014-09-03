>all_greet_prog 100
if ispc($n)
  zerbert $n
  say Hello there $n, I've been waiting for you.
  say Please have a seat so I may enlighten the past.
  say I know much of of the Legends, History, and Problems of this area.
endif
~
>speech_prog p problems problem lizard lizards lizardmen lizardman
if ispc($n)
  say The lizards to the south are invading our territory.
  say They are trying to keep us from the river.
  sigh
endif
~
>speech_prog p legend legends
if ispc($n)
  emote cocks his head and begins to speak.
  say Legend says....... the spirits of our ancestors live on,
  say concealed in the darkness behind the waterfall to the south.
endif
~
>speech_prog p history histories
if ispc($n)
  emote moves around a bit as if getting comfy.
  say Our history is one of great demise and despair.
  say The people of our tribe are descendants of cave dwellers
  say said to have been the first human race created by the three winds.
  emote pauses to catch his breath.
  say .........
  emote clears his throat and spits a goober.
  say The Three Winds are the gods our people worship.
  say For years our ancestors lived undergroud peacefully for ages.
  say Then the evil beats started to appear, multiplying rapidly.
  say Eventually...our forefathers became almost extinct.
  say They fought bravely, but the evil was to strong and to many.
  say Finally, after years of fighting they sought refuge here.
  sigh
endif
~
>speech_prog p help
if ispc($n)
  say Say the word: 'Legend', 'History', or 'Problem' to hear about them.
  say I don't have all day, now!
endif
~
@

