>greet_prog(100, ch=$n, me=$t) {
  %obj = "pamphlet"
  %abr = "pamphlet"
  
  if(!HasObj(ch, obj)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Aye, so yer craving the blood of the sassanaks
    ' Ye'll see a plenty of whits ye seek
    ' Whits fur ye'll no gin by ye
    give $abr $n
    ]
  }
}
