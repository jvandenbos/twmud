>greet_prog(100, ch=$n, me=$t) {
  %obj = 6843
  
  // Check to see if they are already a guard.
  if(!HasObj(ch, obj)) {
   DoWith(me)
    {
     [
      emote motions for you to wait a moment.
      sigh
      'I know who you are $ch, our spies are everywhere.
      'Yes or no question, are you here to join our guard force?
      'If not, then please leave.
     ]
    }
  }

  if(HasObj(ch, obj)) {
   DoWith(me)
    {
     [
     smile $ch
     'Hello again!  How is Ice Haven treating you?
     ]
    }
  }
}

>speech_prog(yes, ch=$n, me=$t, sp=$s) {
  %obj = 6843

  if(!HasObj(ch, obj)) {
    LoadObjOnChar(me, obj)
    DoWith(me) {
       [
        smile
        'Welcome to the team.
        'Here is your badge of authority, keep it on you at all times.
        give ice-haven-guard-badge $ch
        'From now on, please count Ice Haven as your home.
       ]
    }
  }
}
