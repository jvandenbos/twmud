#include ../include/num
#include ../include/string

>command_prog(tasks, ch=$n) {
  if(GetTeam(ch) != TeamState()) {
    return 0
  }

  %i = 0
  SendToChar("Num      Name              Task", ch);
  ForEachMobInZone("mob", GetZone(GetInRoom(ch))) {
    if(IsTeamHunter(mob, ch)) {
      %i = i+1
      
      %job = GetVar(mob, "hunt")
      if(!job) {
        log("No job found on a hunter! (" + GetName(mob) + ")")
	continue
      }
      
      %st1 = CenterPad(Str(i), 3)
      %st2 = CenterPad(GetName(mob), 15)
      %st3 = ""
      if(job == 1) {
	%st3 = "Defend "+TeamColor()+" Flag"
      }
      if(job == 2) {
        %st3 = "Attack "+EnemyColor()+" Flag"
      }
      if(job == 3) {
        %st3 = "Return to Base"
	if(HasObj(mob, TeamFlagName())) {
	  %st3 = st3 + " (our flag)"
	}
	if(HasObj(mob, EnemyFlagName())) {
	  %st3 = st3 + " (enemy flag)"
	}
      }
      if(job == 4) {
        %target = GetVar(mob, "hunt_target")
	if(!target) {
	  %st3 = "Hunting Down... No one?"
        } else {
	  %st3 = "Hunting Down "+target
	}
      }
      %st3 = CenterPad(st3, 20)
      
      SendToChar(st1+" "+st2+" "+st3, ch)
    }
  }
  
  return 1
}

>command_prog(settask, ch=$n, arg=$a, cmd=$s) {
  if(GetTeam(ch) != TeamState()) {
    return 0
  }
  
  while(Left(arg,1) == " ") {
    %arg = right(arg,strlen(arg)-1)
  }
  
  if(cmd == "s") {
    return 0
  }
  
  if(arg == "") {
    MakeString("st") {
[   Syntax for controlling hunters is:
      $CBsettask num defend$CN        - Defend your flag
      $CBsettask num attack$CN        - Attack enemy flag-carrier
      $CBsettask num return$CN        - Defend at scorekeeper
      $CBsettask num kill name$CN     - Seek & Destroy <name>
      ]
    }
    
    SendToChar(st, ch)
    return 1
  }
  
  %spc = InStr(arg, " ", 0)
  if(spc < 0) {
    DoWith(ch) {
      [
      settask
      ]
    }
    return 1
  }
  
  %n = Left(arg, spc)
  %arg = Right(arg, Strlen(arg)-spc+1)
  
  if(!IsNumber(n)) {
    DoWith(ch) {
      [
      settask
      ]
    }
    return 1
  }
  
  %n = Num(n)
  %mob = GetTargetHunter(n,ch)
  %n = Str(n)
  if(!mob) {
    SendToChar("That is an illegal number for a hunter. Use 'tasks' to view your various hunters.", ch)
    return 1
  }
  
  if(ToLower(arg) == "defend") {
    SetVar(mob, "hunt", 1)
    SendToChar("Hunter #"+N+" is now going to find & defend your flag.", ch)
    return 1
  }
  if(ToLower(arg) == "attack") {
    SetVar(mob, "hunt", 2)
    SendToChar("Hunter #"+N+" is now going to find & attack the enemy flag.", ch)
    return 1
  }
  if(ToLower(arg) == "return") {
    SetVar(mob, "hunt", 3)
    SendToChar("Hunter #"+N+" is now going to return to base.", ch)
    return 1
  }
  
  if(Left(ToLower(arg),4) == "kill") {
    %arg = Right(ToLower(arg),Strlen(arg)-5)
    %target = GetChar(arg)
    if(!target) {
      SendToChar("That is an illegal name to hunt down.", ch)
      return 1
    }
    if(GetZone(GetInRoom(target)) != GetZone(GetInRoom(ch))) {
      SendToChar("You can only kill mobs in this zone...", ch)
      return 1
    }
    if(GetTeam(target) == TeamState()) {
      SendToChar("But "+HsSh(target)+" is on your team! You don't want to kill "+HmHr(target)+".", ch)
      return 1
    }
    %target = GetName(target)
    SetVar(mob, "hunt", 4)
    SetVar(mob, "hunt_target", target)
    SendToChar("Hunter #"+N+" is now going to hunt & kill "+target+".", ch)
    return 1
  }
  
  SendToChar("Illegal Command: " + arg + "'", ch);
  return 1
}

>GetTargetHunter(n, ch) {
  %i = 0
  %themob = GetChar("")
  ForEachMobInZone("mob", GetZone(GetInRoom(ch))) {
    if(IsTeamHunter(mob, ch)) {
      %i = i+1
      
      if(i == n) {
        %themob = mob
	break
      }
    }
  }
  
  return themob
}
