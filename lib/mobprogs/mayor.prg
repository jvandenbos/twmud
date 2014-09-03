>all_greet_prog 100
  if ispc($n)
    mpsend here The mayor stops pacing and looks at you as you enter.
    say Thank goodness my plea for help has finally reached someone.
    say Those evil creatures could'nt stop at just stealing cows.
    say A week ago they actually assualted me and stole my ring of office.
    say Please, you must get my ring back and destroy these evil creatures.
    say Give me back the ring and I'll reward you.
  endif
~
>give_prog ring mayor saddlestream
  if ispc($n)
    junk ring
    say Thanks the gods that you have returned safely.
    say Our village don't have much gold but please take this as a sign of our thanks.
    drop treasure
    say Please, take all that you can fit in your pockets!
    say You will always be a welcome guest here in Saddlestream $n.
  endif
~
@
