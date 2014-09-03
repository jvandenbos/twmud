>command_prog(list, ch=$n, mob=$t) {
   MakeString("st") {
   [   Available spells and the required command are as follows:
	  Healing (heal)
          Sanctuary (sanc)
          Remove Poison(removepoison)
          Remove Paralysis (removepara)
          Heroes Feast (heroes)
          Strength (strength)
          Bless (bless)
          Refresh (refresh)
          Mana (mana)
          True Sight (truesight)
          Regeneration (regen)
          Succor (succor)
          Empathic Heal (empathic)
          Group Recall (grouprecall)
          Group Armor (grouparmor)
          Group Sight (groupsight)
          Ray of Purification (ray)
       All spells will be cast on the master only.
   ]
    }
    
    SendToChar(st, ch)
    return 0
}

>command_prog(heal, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'heal' $n
  ]
}

>command_prog(sanc, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'sanc' $n
  ]
}

>command_prog(removepoison, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'remove poison' $n
  ]
}

>command_prog(removepara, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'remove para' $n
  ]
}

>command_prog(heroes, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'heroes' $n
  ]
}

>command_prog(strength, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'str' $n
  ]
}

>command_prog(bless, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'bless' $n
  ]
}

>command_prog(refresh, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'refresh' $n
  ]
}

>command_prog(mana, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'mana' $n
  ]
}

>command_prog(truesight, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'true sight' $n
  ]
}

>command_prog(regen, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'regen' $n
  ]
}


>command_prog(succor, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'succor'
    give recall $n
  ]
}

>command_prog(empathic, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'empathic heal' $n
  ]
}

>command_prog(grouprecall, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'group recall'
  ]
}

>command_prog(grouparmor, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'group armor' $n
  ]
}

>command_prog(groupsight, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'group sight' $n
  ]
}

>command_prog(ray, ch=$n, mob=$t) {
  %n = StringName(GetName(ch))
  [
    cas 'ray' $n
  ]
}
