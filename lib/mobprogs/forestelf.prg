>all_greet_prog 100
 if ispc($n)
say Beware $n
say Beyond this point lies the island of Zy'Tul, home to the beholders.
say They are led by the vile Hive Mother, Thracdanilth.
endif
~
>speech_prog beholder
 if ispc ($n)
say A beholder is a large sinister monster... beared of many eyes.
say They destroy colonies of many different races.
endif
~
>speech_prog spirit
 if ispc ($n)
say Yes.  When I lived, I tried to rid the land of these abominations...
say Alas, the wizard Lomajis betrayed me and took my blade.
endif
~
>speech_prog blade
 if ispc ($n)
say Twas a gift from my father before he died...
emote moans at the mention of the blade briefly, before composing himself.
say Now I am bound to this mortal coil, until I have been reunited with it...
endif
~
>speech_prog lomajis
 if ispc ($n)
say Lomajis was a great wizard who lived within Sanctuary long ago.
say Ever fascinated by the beholders, he was susceptible to their promises of power.
say Susceptible enough to betray me and bind my soul to the base of their temple.
say Now he resides within the halls of the tainted fortress, performing research
say for the hive mother, Thracdanilith.
endif
~
>speech_prog thracdanilth
 if ispc ($n)
emote looks upon you wearily.
say Beware the hive mother, mortal.  She is one of them, and she is beyond them.
say Her will is the Beholders ends, and without her, they have nothing.
say Only in her death will Zy'Tul be cleansed.
emote shakes his head.
say But only a fool would dare ensure such a notion come to fruition. 
endif
~
>speech_prog zy'tul
 if ispc ($n)
say Zy'Tul is a larger continent that rests within the sky.
say Rife with lush forests and mountains, the temple above us is only a small
say piece that was broken off long ago.
emote shakes his head.
say I can only hope and pray it was done so before the rest of the continent was
say corrupted by the vile creatures that inhabit the temple above us.  
endif
~
>give_prog tyrant slayer beholder
if ispc($n)
get slayer
say Thank you, $n!
say I may finally rest knowing you have avenged my spirit.
say Please take this.
give talisman $n
say This talisman will empower your strikes against the forces of evil.
endif
~
@

 
