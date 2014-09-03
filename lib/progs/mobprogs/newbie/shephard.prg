>greet_prog(101, ch=$n, me=$t) {
  %obj = 1802
  %abr = "blade"
  %level = GetFlags(ch, "level")
  %exp = GetFlags(ch, "EXP")
  
  if(!HasObj(ch, obj) && (level <= 3) && (exp <= 1)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Greetings!  You need a weapon? Take this blade just in case.
    ' Twas a gift from a friendly God that aided me in days gone by.
    ' WIELD it and use it well my friend.
    give $abr $n
    ]
  }
}
