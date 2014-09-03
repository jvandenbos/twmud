#include capture_flag/flag_commands

>fight_prog(100) {
  [
  mpset fighting self
  ]
}

>give_prog(red-flag, me=$t, ch=$n) {
  if(TeamState() == "blue") {
    %sc = GetVar(me, "score")
  
    if(!sc) {
      %sc = 0
    }

    if(IsOnTeam(ch)) {
      %mob = GetChar("red-hunter")
      %ChN = GetShort(ch)
      [
      holler Blue Team gets 1 point! ($ChN scored)
      mpat red-hunter give flag red-hunter
      mpat red-hunter drop flag
      ]
      SetVar(mob, "hunt", 3)
      SendCharToRoom(mob, 33300)
      SetVar(me, "score", sc+1)
    } else {
      [
      say You're the wrong team. You want to go back to your area...
      ]
    }
  } else {
    [
    tell $n I don't want this...
    ]
  }
  
  [
  drop flag
  ]
}

>give_prog(blue-flag, ch=$n, me=$t) {
  if(TeamState() == "red") {
    %sc = GetVar(me, "score")
  
    if(!sc) {
      %sc = 0
    }

    if(IsOnTeam(ch)) {
      %ChN = GetShort(ch)
      %mob = GetChar("blue-hunter")
      [
      holler Red Team gets 1 point! ($ChN scored)
      mpat blue-hunter give flag blue-hunter
      mpat blue-hunter drop flag
      ]
      SetVar(mob, "hunt", 3)
      SendCharToRoom(mob, 33338)
      SetVar(me, "score", sc+1)
    } else {
      [
      say You're the wrong team. You want to go back to your area...
      ]
    }
  } else {
    [
    tell $n I don't want this...
    ]
  }
  
  [
  drop flag
  ]
}
