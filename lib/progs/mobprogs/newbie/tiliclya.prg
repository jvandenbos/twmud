>all_greet_prog(100, ch=$n, me=$t) {
  %obj = 3052
  %abr = "recall"
  %level = GetFlags(ch, "level")
  
  if(!HasObj(ch, obj) && (level <= 10)) {
    LoadObjOnChar(me, obj)
    %n = StringName(GetName(ch))
    [
    ' Welcome to Thieves World traveller
    ' Take this scroll and should you get lost or desire a hasty return to the City just RECITE RECALL
    ' Now, go DOWN and Evenita will guide you further.
    ' I do not know what this means, but a God has told me to say the following
    ' If you wish to you may visit our... now what was it called?
    em looks thoughtful for a moment
    ' ... ah yes, our website  at www.thieves-world.com
    em shrug
    give $abr $n
    ]
  }
}
~
@