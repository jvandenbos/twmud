>rand_prog 5
  mpsend here Squir idly watches your feet.
  say Darn it, $r! Could you walk alittle lighter?
  say You're breaking the moss.
~
>greet_prog 90
if name($n) == treyth
  sigh
  tell treyth Hey Treyth, where's that Tacky bot of yours?
endif
~
>greet_prog 90
if name($n) == devl
  grin
  say The devil himself is it?
endif
~
>greet_prog 90
if name($n) == esper
  scream
  say No smurfs! please!!
endif
~
>greet_prog 90
if name($n) == flower
  sigh
  tell flower Hey Flower, give my best to your brother will you?
endif
~
>greet_prog 90
if name($n) == dar
  sigh
  say So you're that lousy fist-fighting mage are ya?
  spit $n
endif
~
>greet_prog 90
if name($n) == quilan
  sigh
  tell quilan Ahh Quilan, how good to see you down here among us mortals.
endif
~
>greet_prog 90
if name($n) == gordo
  sigh
  tell gordo You gonna let Oso have a go at me?
endif
~
>greet_prog 90
if name($n) == quirk
  sigh
  tell quirk I hear you're a hell-of-a tank? A shame I never see you around.
endif
~
>greet_prog 90
if name($n) == mydian
  sigh
  tell mydian So why don't you let Rudiger do the work?
endif
~
>greet_prog 30
if name($n) !/ mriswith
  if isimmort ($n)
    break
  endif
  say Damnit! Why don't anyone ever listen!
  if ispc($n)
    shout Dipsticks like you $n, don't belong in my forest!
  endif
  bash $n
  stand
  cast 'earthquake' $n
  cast 'web' $n
endif
~
>speech_prog help reptiles
  say It is said that these creatures live nearby.
  say I have never seen them myself, but then again, if I had I probably wouldn't be here.
  ponder
~
>rand_prog 10
if isgood($r) and ispc($r)
  look $r
  if level($r) <= 20
    if rand(50)
      say What do you think you're doing, young $r?
      bonk $r
      break
    else
      say This place is not safe for you $r, maybe you wanna recall?
      cast 'succor'
      give recall $r
      nod
      break
    endif
  else
    if level($r) <= 60
      if rand(50)
        say You'd better be careful $r. You might be in over your head.
        smile $r
        break
      else
        say Keep an eye out for them reptiles, $r.
        break
      endif
    else
      if rand(50)
        laugh
        say  So you think you're the one $r? Fine, knock yourself out!
        break
      else
        bow $r
        say You've come to help, $R?
        break
      endif
    endif
  endif
endif
>fight_prog 5
  say Not too bright, are you $n?
~
>fight_prog 90
  cast 'call lightning'
~
>act_prog p panics, and attempts to flee.
if rand(70)
  holler Such wimpish actions are not tolerated in my woods $n!
  mpload m 5401
  mpforce ant follow squir
  mpforce ant guard
  mpload m 5403
  mpforce bear follow squir
  mpforce bear guard
endif
~
>act_prog p is dead! R.I.P.
  rumor Ahahaha! You want a coffin with that, $n?
~
@
