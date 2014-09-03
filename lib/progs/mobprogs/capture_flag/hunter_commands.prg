#include ../include/num

>speech_prog(set levels, me=$t, ch=$n, buf=$s) {
  if(!IsImmortal(ch)) {
    return
  }

  %n = Right(buf, 3)
  
  while((StrLen(n) > 0) && (!IsNumber(n))) {
    %n = right(n, StrLen(n)-1)
  }
  
  if(!StrLen(n)) {
    return
  }
  
  %n = Num(n)

  %n = Max(Min(n,255),1)

  %mob=0
  ForEachMobInZone("mob", GetZone(GetInRoom(me))) {
    if(GetNum(mob) == GetNum(me)) {
      %name = GetUName(mob)
      SetVar(mob, "level", n)
      [
      mpquestup $name $n
      ' Changing $name's level to $n
      ]
    }
  }
}

>speech_prog(generate, me=$t, ch=$n, buf=$s) {
  if(!IsImmortal(ch)) {
    return
  }
  
  %n = Right(buf, 3)
  
  while((StrLen(n) > 0) && (!IsNumber(n))) {
    %n = Right(n, StrLen(n)-1)
  }
  
  if(!StrLen(n)) {
    return
  }
  
  %n = Num(n)
  
  %n = Max(Min(n,20),1)
  
  %Cur = 0
  %mob = 0
  ForEachMobInZone("mob", GetZone(GetInRoom(me))) {
    if(mob == me) {
      continue
    }
  
    if(GetNum(mob) == GetNum(me)) {
      %Cur = Cur + 1
    }
  }
  
  if(Cur >= n) {
    %Count = 1
    ForEachMobInZone("mob", GetZone(GetInRoom(me))) {
      if(mob == me) {
        continue
      }
    
      if(GetNum(mob) == GetNum(me)) {
        if(Count >= n) {
	  %name = GetName(mob)
	  DoAt(mob) {
	    [
	    mppurge $name
	    ]
	  }
	} else {
	  %Count = Count + 1
	}
      }
    }
    
    [
    ' Down to only $Count now.
    ]
  } else {
    %Count = Cur + 1
    while(Count < n) {
      %Count = Count + 1
      %mob = DoLoadDuplicate(me)
      SendCharToRoom(mob, TeamFlagRoom())
    }
    
    [
    ' Up to $Count now.
    ]
  }
}
