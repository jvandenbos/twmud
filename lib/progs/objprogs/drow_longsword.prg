>wear_prog (100, char=$n, obj=$o) {
  %pmsg = "As you wield " + GetShort(obj) + " you immediately feel the urge to spill the blood of the innocent."
  %rmsg = GetShort(char) + " suddenly gets an evil look in his eyes."
  
  SendToRoom(rmsg, GetInRoom(char))
  SendToChar(pmsg, char)
}

>fight_prog (100, 
