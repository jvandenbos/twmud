//This trigger fires if a character dies in the zone
//  if his/her level is <= 10, then they don't
//  make a corpse, but rather do an arena-death.
//  The corresponding trigger is below.

>death_prog(ch=$n) {
  if(GetFlags(ch, "Level") <= 10) {
    SetFlags(ch, "Exp", 0)
    return 1
  }
  
  return 0
}

//This trigger sends a character to room 1846 when
//  they die. It also sets their hps & mana back
//  to max.
>arena_prog(ch=$n) {
  SendCharToRoom(ch, 1846)
  SetFlags(ch, "Hp",   GetFlags(ch, "MaxHP"))
  SetFlags(ch, "Mana", GetFlags(ch, "MaxMana"))
  MakeString("st") {
[$CWAlthough your physical body dies, the gods of the newbie zone have seen fit
to pass you through to their avatar in the world for healing.
]
  }
  SendToChar(st, ch)
  DoWith(ch) {
    [
    look
    ]
  }
  return 1
}
