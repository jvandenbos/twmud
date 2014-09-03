>all_greet_prog(101, ch=$n, me=$t) {
  %obj = 1808
  %abr = "token"
  %level = GetFlags(ch, "level")
  %exp = GetFlags(ch, "EXP")
  
  if(!HasObj(ch, obj) && (level <= 3) && (exp <= 1)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Hello mate. Take this, might help on your walkabout.
    give $abr $n
    ]
      }
 }
