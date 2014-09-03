>greet_prog(100, ch=$n, me=$t) {
  %obj = 6843
  %chName = GetName(ch)

  // Check to see if they are already a guard.
  if(!HasObj(ch, obj)) {
   DoWith(me)
    {
     [
      wipe
      ' Welcome to the Slumbering Wolf Inn.
      ' If you have come for the mead and conversion, then this is the place to be.
      ' If you need a room to sleep for the night, I may be able to find something.
     ]
    }
  }

  if(HasObj(ch, obj)) {
   DoWith(me)
    {
     [
      smile
      ' It's always nice to see the guards taking interest in my humble inn.
      ' If you need a room for the night just give me the word.
      ' Also, so long as you're off duty the bartender is more then happy to serve you.
     ]
    }
   }
}
