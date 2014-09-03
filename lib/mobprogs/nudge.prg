>greet_prog 80
if ispc($n)
  say Good Evenin' Squire!
  nudge $n
  fol $n
if isfollow($i)
if rand(5)
  tell $n Is your wife a goer?  Know what I mean, eh?
  nudge $n
endif
endif
endif
~	    do_say (ch, "Good Evenin' Squire!" , 0 );
	act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	break;
    case NN_FOLLOW:
	switch(number(0,20)) {
	case 0:
	    if (!check_soundproof(ch))
		do_say  (ch, "Is your wife a goer?  Know what I mean, eh?", 0 );
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    break;
	case 1:
	    act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    if (!check_soundproof(ch))
		do_say  (ch, "Say no more!  Say no MORE!", 0);   
	    break;
	case 2:
	    if (!check_soundproof(ch)) {
		do_say  (ch, "You been around, eh?", 0);
		do_say  (ch, "...I mean you've ..... done it, eh?", 0);
	    }
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    break;
	case 3:
	    if (!check_soundproof(ch))
		do_say  (ch, "A nod's as good as a wink to a blind bat, eh?", 0);  
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    break;
	case 4:
	    if (!check_soundproof(ch))
		do_say  (ch, "You're WICKED, eh!  WICKED!", 0);
	    act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
	    act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    act ("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
	    break;
	case 5:
	    if (!check_soundproof(ch))
		do_say  (ch, "Wink. Wink.", 0);
	    break;
	case 6:
	    if (!check_soundproof(ch))
		do_say  (ch, "Nudge. Nudge.", 0);
	    break;
	case 7:
	case 8:
	    ch->act_ptr = NN_STOP;
	    break;
	default:
	    break;
	}
	break;
    case NN_STOP:
	/*
	 **  Stop following
	 */
	if (!check_soundproof(ch))
	    do_say(ch, "Evening, Squire", 0);
	stop_follower(ch);
	ch->act_ptr = NN_LOOSE;
	break;
    default:
	ch->act_ptr = NN_LOOSE;
	break;
    }  
    return TRUE;
}
