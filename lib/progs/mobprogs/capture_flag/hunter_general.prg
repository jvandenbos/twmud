
//Includes
#include capture_flag/hunter_routines
#include capture_flag/hunter_commands


//Functions
>all_greet_prog(100, ch=$n) {
  if(HasObj(ch, TeamFlagName())) {
    if((!IsOnTeam(ch)) && (!IsImmortal(ch))) {
      [
      say You've taken our flag! Die $ch!
      kill $ch
      ]
    }
  }
}

>rand_prog(100, me=$t) {
  %hunt = GetVar(me, "hunt")
  %target = GetVar(me, "hunt_target")
  
  if(!hunt || !target) {
    return
  }
  
  if(hunt != 4) {
    SetVar(me, "hunt_target", "")
    return
  }
  
  %mob = GetChar(target)
  if(!mob) {
    //SetVar(me, "hunt_target", "")
    //SetVar(me, "hunt", 2)
    return
  }
  
  if(GetInRoom(mob) == GetInRoom(me)) {
    %nm = StringName(GetName(mob))
    [
    ' I have been sent to kill you $mob, and will take much pleasure in doing so!
    cac
    kill $nm
    ]
  }
}

>rand_prog(100, rnd=$r, me=$t) {
  if(!rnd) {
    return
  }

  if(HasObj(rnd, TeamFlagName())) {
    if((!IsOnTeam(rnd)) && (!IsImmortal(rnd))) {
      [
      ' You've got our flag $rnd! I shall have to take it back myself!
      kill $rnd
      ]
    }
  }
  if(HasObj(rnd, EnemyFlagName())) {
    if((!IsOnTeam(rnd)) && (!IsImmortal(rnd))) {
      [
      ' $rnd! You're protecting your team's flag! I will kill for that!
      kill $rnd
      ]
      //SetVar(me, "hunt", 2)
    }
  }
  if(HasObj(me, TeamFlagName()) || HasObj(me, EnemyFlagName())) {
    SetVar(me, "hunt", 3)
  } else {
    //choose a random hunt value (protect, or assault)
    %hunt = GetVar(me, "hunt")
    if(!hunt) {
      %hunt = Number(1,2)
    }
    SetVar(me, "hunt", hunt)
  }
}

>rand_prog(100, me=$t) {
  if(!IsOnTeam(me)) {
    SetVar(me, "ctf_team", TeamState())
  }

  if(!GetVar(me, "level")) {
    SetVar(me, "level", 50)
  }
  
  [
  mpset fighting self
  ]
}

>fight_prog(100, me=$t, agg=$n) {
  if(IsOnTeam(agg)) {
    [
    mpset fighting self
    ' I am sorry for fighting you $agg... it will not happen again.
    ' I really have no idea how that could have happened...
    boggle
    ]
    return
  }
  if(HasObj(me, EnemyFlagName())) {
    [
    ' NO! You can't have this flag! It's mine I tell you, all mine!
    scream
    retreat n
    retreat e
    retreat w
    ]
  }
}

>death_prog(100, me=$t) {
  [
  ' Grrr. You may have beaten me now, but I'll be back!
  groan
  ]
  %mob = DoLoadDuplicate(me)
  
  SendCharToRoom(mob, TeamFlagRoom())
  SetVar(mob, "level", GetVar(mob, "level")+1)
}

>give_prog(all, ch=$n) {
  if((!IsImmortal(ch)) && !(IsNPC(ch))) {
    [
    ' A clever one you are...but I don't take gifts!
    drop all
    ]
  }
}

>speech_prog(help, ch=$n) {
  if(IsFighting(ch)) {
    if(IsOnTeam(ch)) {
      [
      ' AT THE RESQUE!!!
      fol ch
      guard on
      cas 'lava'
      ]
    }
  }
}
