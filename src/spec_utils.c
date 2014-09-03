#include "config.h"

#include <stdio.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include <string.h>
#include "structs.h"
#include "utils.h"
#include "constants.h"
#include "db.h"
#include "comm.h"
#include "find.h"
#include "multiclass.h"
#include "act.h"
#include "util_num.h"

char *how_good(int percent)
{
    static char buf[256];
  
    if (percent == 0)
	strcpy(buf, " [        ]");
    else if (percent <= 10)
	strcpy(buf, " [$Ck*$CN       ]");
    else if (percent <= 20)
	strcpy(buf, " [$Ck*$Cr*$CN      ]");
    else if (percent <= 40)
	strcpy(buf, " [$Ck*$Cr*$Cy*$CN     ]");
    else if (percent <= 55)
	strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$CN    ]");
    else if (percent <= 70)
	strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$CN   ]");
    else if (percent <= 80)
	strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$CN  ]");
    else if (percent <= 90)
	strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$CN ]");
    else if (percent <= 100)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN]");
    else if (percent <= 115)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [        ]");
    else if (percent <= 130)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$CN       ]");
    else if (percent <= 145)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$CN      ]");
    else if (percent <= 160)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$CN     ]");
    else if (percent <= 175)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$Cc*$CN    ]");
    else if (percent <= 190)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$Cc*$Cg*$CN   ]");
    else if (percent <= 210)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$Cc*$Cg*$Cy*$CN  ]");
    else if (percent <= 230)
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$Cc*$Cg*$Cy*$Cr*$CN ]");
    else
        strcpy(buf, " [$Ck*$Cr*$Cy*$Cg*$Cc*$Cb*$Cm*$Cw*$CN] [$Cw*$Cm*$Cb*$Cc*$Cg*$Cy*$Cr*$Ck*$CN]");
   
    return (buf);
}

struct char_data *FindMobInRoomWithFunction(int room, spec_proc_func func)
{
    struct char_data *temp_char, *targ;

    targ = 0;

    if (room > NOWHERE) {
	for (temp_char = real_roomp(room)->people; (!targ) && (temp_char); 
	     temp_char = temp_char->next_in_room)
	    if (IS_MOB(temp_char))
		if (mob_index[temp_char->nr].func == func)
		    targ = temp_char;

    } else {
	return(0);
    }

    return(targ);

}

/* ********************************************************************
*  General special procedures for mobiles                             *
***********************************************************************/

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data *npc, char *cmd, int next_line,
                 int *cur_line, void **thing)
{
    bool ok;

    if (GET_POS(npc) == POSITION_FIGHTING)
	return;

    ok = TRUE;

    switch (*cmd) {

    case 'G' :
	*cur_line = next_line;
	return;

    case 'g' :
	*cur_line += next_line;
	return;

    case 'e' :
	act(cmd+1, FALSE, npc, (struct obj_data*) *thing, *thing, TO_ROOM);
	break;

    case 'E' :
	act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
	break;

    case 'B' :
	act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
	break;

    case 'm' :
	do_move(npc, "", *(cmd+1)-'0'+1);
	break;

    case 'w' :
	if (GET_POS(npc) != POSITION_SLEEPING)
	    ok = FALSE;
	else
	    GET_POS(npc) = POSITION_STANDING;
	break;

    case 's' :
	if (GET_POS(npc) <= POSITION_SLEEPING)
	    ok = FALSE;
	else
	    GET_POS(npc) = POSITION_SLEEPING;
	break;

    case 'c' :			/* Find char in room */
	*thing = get_char_room_vis(npc, cmd+1);
	ok = (*thing != 0);
	break;

    case 'o' :			/* Find object in room */
	*thing = get_obj_in_list_vis(npc, cmd+1, real_roomp(npc->in_room)->contents);
	ok = (*thing != 0);
	break;

    case 'r' :			/* Test if in a certain room */
	ok = (npc->in_room == atol(cmd+1));
	break;

    case 'O' :			/* Open something */
	do_open(npc, cmd+1, 0);
	break;

    case 'C' :			/* Close something */
	do_close(npc, cmd+1, 0);
	break;

    case 'L' :			/* Lock something  */
	do_lock(npc, cmd+1, 0);
	break;

    case 'U' :			/* UnLock something  */
	do_unlock(npc, cmd+1, 0);
	break;

    case '?' :			/* Test a random number */
	if (atoi(cmd+1) <= number(1,100))
	    ok = FALSE;
	break;

    default:
	break;
    }				/* End Switch */

    if (ok)
	(*cur_line)++;
    else
	(*cur_line) += next_line;
}

void npc_steal(struct char_data *ch,struct char_data *victim)
{
  int gold;
  
  if(IS_NPC(victim)) return;
  if(GetMaxLevel(victim)>MAX_MORT) return;

  if (AWAKE(victim) && (number(0,GetMaxLevel(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
    act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

