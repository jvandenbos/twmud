#include ../include/capture_flag/general
#include ../include/defines

//This trigger fires if a character dies in the zone
//  if his/her level is <= 10, then they don't
//  make a corpse, but rather do an arena-death.
//  The corresponding trigger is below.

>is_ctf(ch) {
  %rm = GetInRoom(ch)
  %rmflags = GetFlags(rm, "Flags")
  %rm = GetNum(rm)
  
  if((rm >= 33300) && (rm <= 33399) && (rmflags & ARENA)) {
    return 1
  }
  
  return 0
}

>death_prog(ch=$n) {
  return is_ctf(ch)
}

//This trigger sends a character to room 1846 when
//  they die. It also sets their hps & mana back
//  to max.
>arena_prog(ch=$n) {
  if(!is_ctf(ch)) {
    return 0
  }

  %tm = GetTeam(ch)
  if(!tm) {
    return 0
  }
  
  if(tm == "red") {
    SendCharToRoom(ch, 33397)
  } else {
    SendCharToRoom(ch, 33398)
  }

  SetFlags(ch, "Hp",   GetFlags(ch, "MaxHP"))
  SetFlags(ch, "Mana", GetFlags(ch, "MaxMana"))
  SetFlags(ch, "Move", GetFlags(ch, "MaxMove"))

  DoWith(ch) {
    [
    look
    ]
  }
  return 1
}
