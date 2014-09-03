>wear_prog (100, obj=$o, char=$n) {
  %wearmsg = "As you wield " + GetShort(obj) + " a dark aura surrouds your body."
  %wearmsg2 = "As " + GetShort(char) + " wields " + GetShort(obj) + " a dark aura surrounds " + HsHr(char) + " body."

  SendToChar(wearmsg, char)
  SendToRoomExcept(wearmsg2, GetInRoom(char), char)
}

>remove_prog (100, obj=$o, char=$n) {
  %remmsg = "As you remove " + GetShort(obj) + " the dark aura around your body dissipates."
  %remmsg2 = "As " + GetShort(char) + " removes " + GetShort(obj) + " the dark aura around " + HsHr(char) + " body dissipates."
  SendToChar(remmsg, char)
  SendToRoomExcept(remmsg2, GetInRoom(char), char)
}

>fight_prog (100, obj=$o, char=$n, enemy=$t) {
  %fightmsg1 = "Your " + GetShort(obj) + " suddenly glows with an evil "
  %fightmsg1 = fightmsg1 + "dark aura, enveloping " + GetShort(enemy) + "!"

  %fightmsg2 = GetShort(char) + "'s " + GetShort(obj) + " suddenly glows "
  %fightmsg2 = fightmsg2 + "with an evil dark aura, enveloping "
  %fightmsg2 = fightmsg2 + GetShort(enemy) + "!"

  %fightmsg3 = GetShort(char) + "'s " + GetShort(obj) + " suddenly glows "
  %fightmsg3 = fightmsg3 + "with an evil dark aura, enveloping you!"

  SendToChar("$Cr"+fightmsg1+"$CN", char)
  SendToChar("$Cr"+fightmsg3+"$CN", enemy)
  SendToRoomExceptTwo("$Cr"+fightmsg2+"$CN", GetInRoom(char), char, enemy)
}
