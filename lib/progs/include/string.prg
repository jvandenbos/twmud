//String.prg functions:
//  LeftPad - Returns a string of len length, with spaces to the left
//  RightPad - Same as LeftPad, but on the right
//  CenterPad - Same as left & right, but equally on both sides
//  LF - Returns a Line Feed. Simple new line.
//  is_abbrev - Returns whether 
//  Trim - removes the whitespace on either side of the string


>LeftPad(st, len) {
   if(!st) 
   {
     %st = ""
   }
   while(StrLen(st)<len) 
   {
     %st = " " + st
   }
   return st
 }

>RightPad(st, len) {
   if(!st) 
   {
     %st = ""
   }
   while(StrLen(st)<len) 
   {
     %st = st + " "
   }
   return st
 }

>CenterPad(st, len) {
   if(!st) 
   {
     %st = ""
   }
   %i=0
   while(StrLen(st)<len) {
     if(i%2) 
     {
       %st = " " + st
     } 
     else 
     {
       %st = st + " "
     }
     %i=i+1
   }
   return st
 }



>LF() {
   MakeString("st") {
[
]
  }
  return st
}




>is_abbrev(st, st2) { 
   %len1 = strlen(st)
   %len2 = strlen(st2)
   %i = 0
   while(i < len1) {
     if((i > len2) || (mid(st, i, 1) != mid(st2,i,1))) 
     {
       return 0
     }
     %i += 1
   }
   return 1
 }

>Trim(st) {
   while(Left(st, 1) == " ") 
   {
     %st = Right(st, Strlen(st)-1)
   }
   while(Right(st, 1) == " ") 
   {
     %st = Left(st, Strlen(st)-1)
   }
   return st
 }
