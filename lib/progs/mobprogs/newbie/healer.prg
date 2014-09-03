>greet_prog(100, ch=$n) {
  [
  ' If you have need of our healing powers,
  ' GROVEL or SAY HEAL.
  ]
}

>command_prog(grovel, ch=$n, mob=$t) {
  if((GetNum(mob) == 1807) && level_check(ch)) {
    heal_func(ch,mob)
    [
    ' May the Gods be with you.
    ' If ever you require our healing services again,
    ' just GROVEL before the Avatar once again.
    ]
  }
  
  return 0
}

>speech_prog(heal, ch=$n, mob=$t) {
  if((GetNum(mob) == 1808) && level_check(ch)) {
    heal_func(ch,mob)
    [
    ' May my protection help guide you.
    ]
  }
}

>rand_prog(5, ch=$n, mob=$t) {
  if(level_check(ch)) {
    heal_func(ch,mob)
  }
}

>heal_func(ch,me) {
  %n = StringName(GetName(ch))
  
  [
  cas 'heal' $n
  cas 'sanc' $n
  cas 'bless' $n
  cas 'armor' $n
  cas 'refresh' $n
  cas 'str' $n
  ]
  SetFlags(me, "Mana", GetFlags(me, "MaxMana"))
}

>level_check(ch) {
  return (GetFlags(ch, "Level") < 11)
}
