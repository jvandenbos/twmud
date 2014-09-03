>greet_prog 50
  sigh
  say Oh. I guess you'll be staying for lunch too, $n?
  emote reaches into the pantry for some more tea cakes.
~
>fight_prog 15
if isinroom(gandalf)
  say Gandalf!  Save me!
  mpforce gandalf rescue $i
  break
else
  shout Gandalf!  How will I ever complete your quest!  Help me!
  mpload m 791
  mpsend here Gandalf arrives in an orange flash of light!
  mpsend here Gandalf looks upon you with disdain
  mpsend here You feel a curse slowly stretching through your veins...
endif
~
@

