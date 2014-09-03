>speech_prog voices voice
if ispc($n)
  say Well....
  say I was investigating the tombs one day and I found a stone door.
  say I went to force it open and a voice told me to leave and never re-enter the the tombs.
  say I have never gone back, or will I.
  shrug
endif
~
>speech_prog tomb tombs crypt crypts
if ispc($n)
  say The tomb is located south of here, several leagues away.
  say Its quite interesting really, but I wouldn't suggest going inside, because of the voices.
  emote seems quite uncomfortable.
endif
~
>speech_prog discovery discoveries advance advances road roads wall walls ruin ruins
if ispc($n)
  say Well, $n, their creations speak for themselves.
  say I've witnessed their engineering of roads and walls for one,
  say they are constructed to withstand the mightiest of natures ravages.
  say Go visit the tomb to the south, You'll see what I mean.
endif
~
>speech_prog ch'tar valley chtar ch-tar
if ispc($n)
  say I'm an anthropologist, the Ch'tar peoples were some of the most advanced peoples I've ever studied.
  say Their advances were far beyond those of our own, in fact for a non-magical people they made amazing discoveries.
endif
~
>greet_prog 50
if ispc($n)
  say Why hello, so long since I've had strangers come here...
  sigh
  say Welcome to the Valley of the Ch'tar.
endif
~
@
