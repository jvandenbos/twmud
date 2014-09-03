>greet_prog(100, ch=$n, me=$t) {
  %obj = 1825
  %abr = "eye"
  
  if(!HasObj(ch, obj)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Welcome friend. The choice of path you take from here is yours.
    ' To the SOUTH is the reknowned Sanctuary Academy. There you may learn the ways of this world.
    ' Directly DOWN is the City of Sanctuary. Beware the Thieves of Thieves World.
    ' I give you this light to guide you and light your way. Use it well.
    give $abr $n
    ]
  }
}
