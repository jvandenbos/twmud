>give_prog 100
if ispc($n)
  if rand(50)
    say This wasn't filled out correctly, $n.
    say You'd better redo it.
  else
    emote quickly glances at the $O.
    say This looks good.  I'll get to it later.
    emote puts the $O at the bottom of a large pile and continues with his paperwork.
  endif
endif
~
>rand_prog 15
if ispc($r)
  if rand(50)
    say A soldier's work is never done.
    emote continues with his paperwork.
  else
    if rand(50)
      say Sorry, $r.  How can I help you today?
    else
      say File your report in triplicate.
      mpload o 3847
      give paper $r
      say Sorry... someone stole my spare pen.
    endif
  endif
endif
~
>speech_prog pen pencil quill
if ispc($n)
  if rand(50)
    if rand(50)
      say I told you already, someone took it!
    else
      say I'm feeling very insecure without that pen.
      cry $i
    endif
  endif
  if level($n) <= 20 or ispeaceroom()
    mpsend char $n $I grabs you by the neck.
    mpsend room $n $I grabs $n by the neck.
    say Ah, you're not worth the trouble.
    say Get outta here, ya bum... before I change my mind.
  else
    say That's it! I'm sick of people picking on me!
    say Get him, guys!!!
    mpforce cityguard kill $n
    mpforce 2.cityguard kill $n
    mpforce 3.cityguard kill $n
    mpforce 4.cityguard kill $n
    assist cityguard
  endif
endif
~
>speech_prog paper parchment
if ispc($n)
  if rand(50)
    say You have some, I gave it to you.
    say If you lost it, it's gone.
  else
    look $n
    say Look buddy, don't get me angry.
  endif
endif
~
>fight_prog 15
  say Well met ruffian...now you shall die!!
  bash
~
@
