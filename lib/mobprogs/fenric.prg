>bribe_prog 99
if ispc($n)
  lgate adair
  smile
  tell $n There ya go.
endif
~
>greet_prog 85
if ispc($n)
  if rand(50)
    say Welcome, $n
    mpsend here Currently I am offering gateways to Mordilnia for 100 coins.
    mpsend here Give me the coins, and the gateway is yours.
  else
    say Mordilnia is beautiful this time of year!
    daydream
  endif
endif
~
>fight_prog 50
  mindblast $n
  mindblast $n
~
@
