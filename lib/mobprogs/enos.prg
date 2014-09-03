>all_greet_prog 100
if ispc($r)
  say Ah, you have defeated my pets... faithful to the end, they were.
  sniff
  say Perhaps you care to challenge me now?
endif
~
>rand_prog 50
if ispc($r)
  sneer $r
  cast 'para' $r
  cast 'fireball' $r
endif
~
>fight_prog 40
  cast 'para'
  cast 'fireball'
~
>death_prog 100
  shout My treasure will NOT be left unguarded!
  mptransfer wyrm
  mpforce wyrm kill $n
  scream
~
@

