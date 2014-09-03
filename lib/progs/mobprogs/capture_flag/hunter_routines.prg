>rand_prog(100, me=$t, rnd=$r) {
  if(HasObj(me, TeamFlagName())) {
    if(rnd && IsOnTeam(rnd)) {
      if(!IsNPC(rnd)) {
        %TFN = TeamFlagName()
        [
        ' Here. I believe -you- are the one who should hold this.
        give $TFN $rnd
	' Now I'm going to find their damn flag...
	]
	%obj = GetObj(EnemyFlagName())
	SetHuntObj(me, obj)
	SetVar(me, "hunt", 2)
        return
      }
    }
  }
  
  %hunt = GetVar(me, "hunt")
  %prev = GetVar(me, "prev_hunt")
  
  if(!hunt) {
    %hunt = Number(1,2)
  }
  
  SetVar(me, "hunt", hunt)
    
  if(!prev) {
    %prev = 0
  }
  
  DoHuntRoutine(me, hunt, prev)

  if(HasObj(me, EnemyFlagName())) {
    if(rnd && IsOnTeam(rnd)) {
      if(!IsNPC(rnd)) {
        %EFN = EnemyFlagName()
        [
        ' Hey! $rnd! It seems as if I've been doing all the work around
	' here. I'll tell you what. You take this flag, and hurry up and
	' score with it! I'll cover your back.
	sigh
	' Kids these days.
	give $EFN $rnd
	fol $rnd
	guard on
	]
	SetVar(me, "hunt", 1)
      }
    }
  }
}

>rand_prog(100, me=$t) {
  %rm = GetInRoom(me)

  if(HasObj(rm, TeamFlagName())) {
    %TFN = TeamFlagName()
    [
    holler I've found our flag! I'm returning to base.
    get $TFN
    ]
    SetVar(me, "hunt", 3)
  }
  if(HasObj(rm, EnemyFlagName())) {
    %EFN = EnemyFlagName()
    [
    rofl
    ' Tee hee hee... I've found their flag! They'll never miss this...
    get $EFN
    ]
    SetVar(me, "hunt", 3)
  }
}

>DoHuntRoutine(me, hunt, prev) {
  if(hunt == 1) {
    //hunt mode 1 = help team flag carrier
    if((hunt != prev) || (!IsTracking(me))) {
      %obj = GetObj(TeamFlagName())
      SetHuntObj(me, obj)
      SetVar(me, "prev_hunt", hunt)
    }
  }

  if(hunt == 2) {
    //hunt mode 2 = find enemy flag
    if((hunt != prev) || (!IsTracking(me))) {
      %obj = GetObj(EnemyFlagName())
      SetHuntObj(me, obj)
      SetVar(me, "prev_hunt", hunt)
    }
  }

  if(hunt == 3) {
    //hunt mode 3 = goto base
    if((hunt != prev) || (!IsTracking(me))) {
      SetHuntRoom(me, TeamFlagRoom())
      SetVar(me, "prev_hunt", hunt)
    }
  }
  
  if(hunt == 4) {
    //hunt mode 4 = seek & destroy!
    if((hunt != prev) || (!IsTracking(me))) {
      %target = GetVar(me, "hunt_target")
      if(!target) {
        log("I'm not hunting anyone in particular! ("+GetUName(me)+")")
	StopTracking(me)
	SetVar(me, "hunt", 2)
	return
      }
      SetHuntChar(me, GetChar(target))
      SetVar(me, "prev_hunt", hunt)
    }
  }
}
