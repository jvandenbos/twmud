>all_greet_prog(100, ch=$n, me=$t)
{
  %numTimes = GetVar(me, "Status")

  if (!numTimes)
   { 
    SetVar(me, "Status", 1)
    %numTimes = 1
   }
    if (numTimes > 0)
     {
      DoWith(me) 
       {
        [
         emote raises his hand and gestures at your party.
         ' It has been nearly a thousand years since mortals last dared set foot here
         ' Are you truely worthy to take the test?
         ' If so than by all means proceed, if not, turn back now.
         ' For as surely as this world is dieing, so will you!
         emote raises his head just enough to see two rows of shining steel fangs.
         ' Those of you that are brave enough to proceed, do so with apprehension.
         ' The inquisitors in the following rooms offer no solace for the weak.
         ' Their test is without mercy for the unknowledgeable.
         ' They will describe to you two beings.
         ' One will be weak in stature, the other feared by all those with something to loose.
         emote leans towards you.
         ' Choose right for the easy kill and great rewards.
         emote gestures passively with an open palm, which than closes into a tight fist!
         ' Choose wrong... and your death cries will be heard around the kingdom!
         smile
         ' There is one other option you may take.
         ' While you may not return back to previous rooms, you may continue onward.
         ' If you can not decide which being to fight, than simply continue down the path.
         ' At the end is your exit from my halls, and your freedom with what ever loot you may find.
         'Good luck for now, you will need it.
         grin
        ]
       }
     %numTimes = numTimes - 1
     SetVar(me, "Status", numTimes)
    }
}
