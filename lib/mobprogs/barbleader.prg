>all_greet_prog 100
 if ispc($n)
say Welcome $n.
say I have a small favor to ask.
mpsend here The barbarians turn and seem to be staring at you now
say A kindom of horrible monsters seem to be gathering underground,
my sources tell me their leader is King Thorn.
say If you can bring me the key he carries and drop it here I will reward
you greatly.
say By the way...make sure your pockets are empty when you bring the key.
say Fair thee well $n.
endif
~
>act_prog p drops Vault Key.
if ispc($n)
cheer
get key
say $n, ...You are truly the hero from the prophecy.
mpload o 160
say Here is your reward.
drop treasure
mpjunk key
say May your journeys be full of fortune $n.
endif
~
@
