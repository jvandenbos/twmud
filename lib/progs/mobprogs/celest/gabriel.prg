>greet_prog(100, ch=$n, me=$t) {
  %obj = 18531
  %chName = GetName(ch)

  // Check to see if they have peters key.
  if(!HasObj(ch, obj)) {
   DoWith(me)
    {
     [
      junk marbles
      emote stops sharpening his axe.
      sigh
      'I wish I could find my Sledgehammer.
      em looks up at you
      ' Oh! Hello, how are you?.
      ' Queen Iris said you might "drop round"
      ' Hope you are having fun.
      em goes back to sharpening his axe.
     ]
    }
  }

  if(HasObj(ch, obj)) {
   DoWith(me)
    {
     [
     junk marbles
     em stops sharpening his axe
     smile $n
     ' Hello again! You haven't found my sledgehammer have you?
     em looks at you
     ' I see you have not. Oh well. Probably shouldn't have loaned it to Phil.
     em goes back to sharpening his axe.
     ]
    }
  }
}
~
