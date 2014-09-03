#include ../include/string

>command_prog(help, ch=$n, arg=$a) {
  while(left(arg,1) == " ") {
    %arg = right(arg, strlen(arg)-1)
  }
  while(right(arg,1) == " ") {
    %arg = left(arg, strlen(arg)-1)
  }
  
  if(is_abbrev(arg,"instructions")) {
    ShowInstructions(ch)
    return 1
  }
  
  if(is_abbrev(arg,"hunters")) {
    ShowHunters(ch)
    return 1
  }
  
  return 0
}

>ShowInstructions(ch) {
  MakeString("st") {
  [
    Ok, here are all of the instructions for the Capture the Flag
    game. To start out, each team has the following things:
      -Flag
      -Scorekeeper
      -Hunter(s)

    First thing's first. Quest Imm chooses two team leaders. Then,
    in an alternating fashion, each leader picks someone to be on
    their team. Once both teams have been picked, the quest imm
    will then open up the door upwards, to the center of the zone.
    From there, your leader will bring you to your flags. If you
    are the red team, your flag is to the southern end of the map,
    while if you are the blue team, your flag is to the very north
    of the map. Once you've all arrived to your flags, you must
    immediately say the word "init". This is of grave importance,
    and the quest god will be EXTREMELY angry, if you do not say
    this, and you may die/lose levels/get meteored depending upon
    how the quest goes. Once you do that, wait until the quest imm
    says to start the quest, and then go for it! General rules are
    as follows:

    -Get your opponent's flag at all costs. If this means by
       killing, raping, burning, stealing, whatever. Do this.
    -Once you do recieve your opponent's flag, return to your
       scorekeeper. When you enter the room, type "give flag team",
       and your team will score a point. Yay.
    -The hunters are there to help your team. Don't attack your
       own team's hunters, as that's not friendly play. Depending
       upon what level the quest imm set the hunters at, it may
       even cost you your life.
    -Feel useless? You're not. Follow your own flag-bearer, at any
       costs, as if he dies, the flag drops to the ground. This is
       BAD, as enemies can pick it up. It is good if you pick up
       your flag, and give it to someone safe ASAP.
    -Hunters got you confused? Type "help hunters" for info
       regarding these little gems.

    Misc:
     -Zone is No-Sneak, and No-Gate. There is no entering, or
       leaving until the quest imm says when.
     -Zone is SAFE. That's right folks, Satire Angry Fetal Elephants.
       Actually, it just means that when you die, you don't lose
       levels, or anything, you repop in a teleporter room for
       your team, full health, with the same shift/polymorph that
       you had on before.
     -Triggers are god-damned annoying, and I'm thinking of not
       allowing them. Hrmm. Don't push me, cause Laguna's
       complaining is making a good point in my mind.
     -No using containers. If I find that a flag has been placed in
       someone's bag, that's immediate dismissal from the quest.

    Above all.... ENJOY! I wrote these routines for your enjoyment.
      -Quilan (Buy my Q-Dogga action figures!)]
  }

  SendToChar(st, ch)
}

>ShowHunters(ch) {
  MakeString("st") {
    [
    Hunters are your friend. They are creatures that have a large
    set of AI code, that essentially do many tasks to help out
    your team. Here's a quick summary of all their functions:

    Defend your flag
      -The hunter will stay by the side of your teammate which has
      your flag, and will protect him if he's ever in a fight.

    Attack other flag
      -The hunter will go assault the other team's flag carrier.

    Get flags
      -If either flag is on the ground (your teams, or opponent's),
      the hunter will pick it up and return to base. If it picks
      up your team's flag, then it will holler, so you'll know
      that it has the flag. When it finds a member of your team,
      then it will give whatever flags it is holding to that
      person.

    Controlling Hunters
      -To control any of your hunters, you can use one of two
      commands: "tasks", or "settask" in your flag room. Here's
      what the commands do:
      
      -Tasks
        This will show you what all of your hunters are currently
	up to.
      
      -Settask
        You use this command to set the different jobs that your
	hunters will be performing.]
  }
  
  SendToChar(st, ch)
}

>command_prog(startq, ch=$n, arg=$a) {
  if(!IsImmortal(ch)) {
    return
  }
  
  DoWith(ch) {
    [
    at 33300 load m red-team
    at 33338 load m blue-team
    load o red-flag
    load o blue-flag
    ]
  }
  
  %n = Num(arg)
  %i = 0
  while(i < n) {
    DoWith(ch) {
      [
      at 33300 load m red-hunter
      at 33338 load m blue-hunter
      ]
    }
    %i = i + 1
  }
}

>command_prog(reset, ch=$n, arg=$a) {
  while(Left(arg, 1) == " ") {
    %arg = Right(arg, strlen(arg)-1)
  }
  
  while(Right(arg, 1) == " ") {
    %arg = Left(arg, strlen(arg)-1)
  }
  
  if(arg == "state") {
    SetVar(ch, "ctf_team", "")
  }
}
