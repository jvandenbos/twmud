>greet_prog 100
if ispc($n)
  say You cannot escape!
  cast 'weakness' $n
  cast 'electrocute' $n
endif
~
>fight_prog 100
  cast 'electrocute'
  bash  
~
>death_prog 100
if ispc($n)  
  growl
  say Damn you $n, Is not over!
  grin
  say I hope you rot in hell.
  say I know where you live!
  emote Drops dead as 4 of its tentacles break off of his body!
  mpload m 32581
  mpforce tentacle kill $n
  mpload m 32581
  mpforce tentacle kill $n
  mpload m 32581
  mpforce tentacle kill $n
  mpload m 32581
  mpforce tentacle kill $n
endif
~
@

