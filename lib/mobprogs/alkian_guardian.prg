>greet_prog 100
if ispc($n)
  smile
  mpsend here Ah, You have found the Ancient Burial grounds.
  mpsend here No one may pass if they have not experienced the world.
  mpsend here Would you like to hear the story of the Alkian race?
  emote smiles as he waits for a reply.
endif
~
>speech_prog yes
if ispc($n)
  say Ahh, good.
  say It may help you on your travels beyond here.
  say What would you like to know about?
endif
~
>speech_prog demonspawn leader demon spawn
if ispc($n)
  say Good question.  The DemonSpawn is the father of all Alkians.
  say He was once the Western Wizard of Neutrality.
  say He has no weakness, but show him respect and he may be easier to beat.
endif
~
>speech_prog master
  say So, you have heard about the Alkian Masters? 
  say They train the Alkian Warriors.  Many possess powerful magic.
  say The imps are there little servents that will give their life for him.
~
>speech_prog journeyman journey
  say The Journeymen where once guardians of the Emperor. 
  say They travel theses lands telling and collecting stories.
  say Though he only carries a shovel, it can leave quite a mark.
~
>speech_prog imp imps
  say Yea, those pesky little critters.  They have practiced on me before.
  say They are quite the little casters, but so focused on one type.
  shake
~
>speech_prog holly
  say Ah what a beautiful young woman she is.  Very pure and holy.
  say I am not sure why she is even here.  Some friends or something.
  boggle
~
>speech_prog no
if ispc($n)
  say Fine, but be warned, they do not like outsiders!
  spit
endif
~
>speech_prog alkian
if ispc($n)
  say They are the strongest warriors of this lan
  say They get strength from thier enemies by draining during battle.
endif
~
>speech_prog help
  say I can tell you a little bit about some of the beings of this land.
  say Just say the name and I will tell you what I can.
  say But understand that some secrets wil stay with me to the end!
~
>speech_prog berserker berserkers
  say Watch out for the berserkers.  They tend to get aid even after they
  are dead.
  say I would suggest being heavily warded while fighting here.
~
>speech_prog grand woman grandwoman
  say Wow, she is so damn HOT!  But becareful of her mystifiing look.
  say Many of the beings in the Astral plane found their way by her commands.
  say She makes Demi look like a weakling.  Imagine that cat fight!
  giggle
  blush
~
@
