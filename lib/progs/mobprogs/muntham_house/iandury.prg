>give_prog(letter-mr-i-dury, me=$t, ch=$n) {

   LoadObjOnChar(me, 36006)

   DoWith(me)
    {
     [
      'Why thank you, I have been waiting for this letter for awhile.
      'Those prefects will learn not to mess with me. Please take these keys for your trouble.
      bow 
      give headmaster-bunch-key-keys $ch
      junk letter-mr-i-dury
     ]
    }
}
