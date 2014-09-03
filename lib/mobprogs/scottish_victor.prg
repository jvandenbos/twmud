>fight_prog 100
  cast 'acid rain'
  cast 'acid rain'
  cast 'dispel magic' $n
~
>death_prog 100
  shout $n, You will pay for this! Attack!
  mptransfer guardian-noble
  mpforce guardian get all
  mpforce guardian kill $n
~
@