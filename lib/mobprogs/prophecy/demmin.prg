>all_greet_prog 50
if ispc($n)
  say So $n, passed the grunts did you?
  chuckle
  say Good job. Too bad you're not my type.
  swear Time to die $n!
  emote summons one of his quads.
  mpload m 5444
  mpforce quad kill $n
endif
~
>fight_prog 5
if ispc($n)
  shout Ooooah, that's more like it $n!
  lunge $n
endif
~
>fight_prog 8
  shout I'm loving every minute $n!
  cast 'blindness' $n
  trip $n
~
>act_prog p panics, and attempts to flee.
if ispc($n)
  mpload o 6783
  shout You little bastard! Get your ass back here, $n!
  throw dagger $n
endif
~
>death_prog 90
    mpsend here In a last desperate effort Demmin calls forth his minions.
    mpload m 5444
    mpload m 5444
    swear Damnit $n! I thought you loved me!
    mpforce quad kill $n
    mpforce 2.quad kill $n
~
@
