#include ../include/string

>SetTeam(char, v) {
  SetVar(char, "ctf_team", v)
}

>speech_prog(init, ch=$n, me=$t, sp=$s) {
  %sp = Trim(sp)

  if(sp != "init") {
    return
  }

  %mps = GetTeam(ch)
  
  if((mps != "red") && (mps != "blue")) {
    %tn = TeamName()
    [
    ' Initiating you to $tn, $ch
    ]
    SetTeam(ch, TeamState())
  } else {
    if(mps == EnemyState()) {
      [
      ' You're the other team $ch!
      laugh $ch
      ]
    }
  }
}

>speech_prog(score, me=$t, sp=$s) {
  %sp = Trim(sp)

  if(sp != "score") {
    return
  }

  %sc_1 = GetVar(me, "score")
  if(TeamState() == "blue") {
    %mob = GetChar("red-team")
  } else {
    %mob = GetChar("blue-team")
  }
  %sc_2 = GetVar(mob, "score")
  
  if(!sc_1) {
    %sc_1 = 0
  }
  
  if(!sc_2) {
    %sc_2 = 0
  }
  
  [
  ' Currently the score is my team: $sc_1, enemy: $sc_2.
  ]
}
