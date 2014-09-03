>all_greet_prog(100, me=$t)
{
 [
  'Are you ready for my question?
  'If so then say askme
  'Note that I will repeat the question 3 times, no more, no less.
 ]
}


>speech_prog(askme, me=$t, ch=$n, text=$s) { 
 %numTimes = GetVar(me, "Status")
  if (!numTimes)
   { 
    SetVar(me, "Status", 3)
    %numTimes = 3
   }
    if (numTimes > 0)
     {
       DoWith(me) 
        {
         [
          emote motions for your party to gather around and listen.
          ' They are both paladins, her (Being1) and him (Being2).
          ' Being1 fights with a weapon, Being2 has no need to.
          ' Being1 is the only one of her kind.
          ' Being2 is one among many.
          ' Make your choice?  Being1 or Being2?
         ]
        }
      %numTimes = numTimes - 1
      SetVar(me, "Status", numTimes)
     } else {
        DoWith(me) {
         [
          'You are SOL! You should have listened!
         ]
        }
       }
      } 
}
>speech_prog(being1, me=$t, ch=$n, text=$s)
{
	DoWith(me)
	{
	 [
    	    mpload mobile 32147 // galahad
         ]
        }
 %aggMob = GetCharInRoom("unknown-being", 8855)
 StartFight(aggMob)
}
 

>speech_prog(being2, me=$t, ch=$n, text=$s)
{
       DoWith(me)
       {
         [
	     mpload mobile 32148 // holly
         ]
       }
 %aggMob = GetCharInRoom("unknown-being", 8855)
 StartFight(aggMob)
}
 

>death_prog(100, me=$t, killer=$n)
{
 %room = GetInRoom(me)
 %newRoom = 8856
 DoWith(me) 
  {
   [
    'On my dying breath, you may continue... well done.
   ]
  }
 TransAllInRoomToNewRoom(room, newRoom)
}


>StartFight(aggMob)
{
 DoWith(aggMob)
 {
  [
   'You have summoned me! Now, prepare to die!
   Cast 'creep'
   ]
 }
}

>TransAllInRoomToNewRoom(room, newRoomNumber)
{
 %tempChar

 ForEachMobInRoom("currentMob", room)
  {
   tempChar = GetCharInRoom(GetVar(aggMob, "currentMob"),room)
   SendCharToRoom(tempChar, newRoomNumber)
  }
}
