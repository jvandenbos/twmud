//Functions

>TeamName() {
  return TeamColor() + " Team"
}

>DoLoadDuplicate(me) {
  DoAt(1000) {
    %MyNum = GetNum(me)
    [
    mpload m $MyNum
    ]

    ForEachMobInRoom("mob", 1000) {
      if(me == mob) {
	continue
      }
      if(GetNum(mob) == MyNum) {
        break
      }
    }
    
    %un = GetUName(mob)
    %lvl = GetVar(me, "level")
    [
    mpquestup $un $lvl
    ]
    SetVar(mob, "level", lvl)
    SetVar(mob, "hunt", GetVar(me, "hunt"))
    SetVar(mob, "hunt_target", GetVar(me, "hunt_target"))
  }
  
  return mob
}

>IsOnTeam(char) {
  %mpstate = GetTeam(char)

  if(mpstate == EnemyState()) {
    return 0
  }
  if(mpstate == TeamState()) {
    return 1
  }
  
  return 0
}

>GetTeam(char) {
  if(char) {
    %mps = GetVar(char, "ctf_team")
    if(!mps) {
      %mps = ""
    }
  }else {
     %mps = ""
  }
   
  return mps
}

>IsTeamHunter(mob, ch) {
  %team = GetTeam(ch)
  if(!team) {
    return 0
 }
  
  %n = GetNum(mob)
  if(((n == 32402) && (team == "red")) || ((n == 32403) && (team == "blue"))) {
   return 1
  }


  return 0
}
