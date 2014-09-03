>all_greet_prog(99, ch=$n, me=$t) {
  %obj = 36003
  if(!IsNpc(ch) && !HasObj(ch, obj)) {
      DoWith(me) {
        [
        'No walking in the halls without a hallpass!
        '$ch, you must be punished!
        hit $ch
        ]
      }
  }
}
>all_greet_prog(99, ch=$n, me=$t) {
  %obj = 36003
  if(!IsNpc(ch) && HasObj(ch, obj)) {
      DoWith(me) {
        [
        growl
        'I see you have a hallpass.
        'Hurry up about your business and get back to class before I decide to kick your butt anyway.
        ]
      }
  }
}
