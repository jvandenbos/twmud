>greet_prog(100, ch=$n, me=$t) {
  %obj = 1825
  %abr = "illumini"
  %level = GetFlags(ch, "level")
  
  if(!HasObj(ch, obj) && (level <= 10)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Welcome friend. You may now choose your path.
    ' If you wish to learn of our world's ways go SOUTH and enter the prestigious Sanctuary Academy
    ' If you wish to enter the City of Sanctuary, go DOWN.
    ' Watch out for the Thieves of Thieves World, they will be watching for you.
    give $abr $n
    ]
  }
}
