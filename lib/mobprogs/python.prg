>greet_prog 80
if ispc($n)
  look $n
  if level($n) <= 10
    if rand(50)
      say Careful, $n.  Don't get yourself into trouble.
      poke $n
    else
      say Remember to use the CONSIDER command before blindly attacking a target such as myself.
    endif
    break
  endif
  if level($n) <= 35
    if rand(50)
      say Are you man enough to take me on, $n?
      emote beckons for you to step closer.
    else
      say Ready to rumble, $n?
      emote cracks his fingers.
    endif
  else
    if rand(50)
      say Get outta here, $n.
    else
      say Begone, $n!
    endif
  endif
endif
~
>rand_prog 20
if ispc($r)
  look $r
  if level($r) <= 10
    if rand(50)
      say Careful, $r.  Don't get yourself into trouble.
      poke $r
    else
      say Remember to use the CONSIDER command before blindly attacking a target such as myself.
    endif
    break
  endif
  if level($r) <= 35
    if rand(50)
      say Are you man enough to take me on, $r?
      emote beckons for you to step closer.
    else
      say Ready to rumble, $r?
      emote cracks his fingers.
    endif
  else
    if rand(50)
      say Get outta here, $r.
    else
      say Begone, $r!
    endif
  endif
endif
~
>fight_prog 40
if rand(50)
  if rand(50)
    say You're doing well, $n.
  else
    say Remember your training.  Try using your skills!
  endif
else
  if rand(50)
    say Nicely executed, $n!
    whistle
  else
    say Remember to look at your target during the fight to know where you stand.
  endif
endif
~
@
