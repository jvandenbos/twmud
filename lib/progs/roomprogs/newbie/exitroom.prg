#include ../include/string

>command_prog(down, ch=$n) {
  %curhome = GetFlags(ch, "Home")
  %level = GetFlags(ch, "Level")
  
  if((curhome == 1800) && (level > 10)) {
    SetFlags(ch, "Home", 3001)
    %st1 = "$CYYou are now powerful enough to leave the academy."
    %st2 = "Good luck on your jouneys!$CN"
    SendToChar(st1 + LF() + st2 + LF(),ch)
  }
  
  return 0
}
