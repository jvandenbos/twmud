#include "config.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if USE_stdlib
#include <stdlib.h>
#endif

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "find.h"
#include "spells.h"
#include "constants.h"
#include "spec.h"
#include "interpreter.h"
#include "area.h"
#include "opinion.h"
#include "fight.h"
#include "race.h"
#include "spelltab.h"
#include "utility.h"
#include "act.h"
#include "spell_procs.h"
#include "multiclass.h"
#include "util_str.h"
#include "handler.h"
#include "spell_util.h"
#include "skills.h"
#include "recept.h"
#include "trap.h"
#include "hero.h"
#include "track.h"
#include "statistic.h"
#include "vnum_mob.h"
#include "periodic.h"
#include "proto.h"
#include "state.h"
#include "cmdtab.h"
#include "engine.h"
#include "scrap.h"

/* Data declarations */
struct social_type {
  char *cmd;
  int next_line;
};

/* forward declarations */
void ShowPracs(struct char_data* ch, int max, int clss);
void PracSpell(struct char_data* ch, int max, int clss, char* arg);
int GainLevel(struct char_data *ch, int clss);
int attack_convict(struct char_data* ch);
void guard_help_call(struct char_data *guard, int level, int town);
int guard_idle_action(struct char_data *guard);

/* special procs for mobs */

SPECIAL(mayor)
{
    static char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static char *path;
    static int index;
    static bool move = FALSE;
    struct char_data *mayor = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (!move) {
	if (time_info.hours == 6) {
	    move = TRUE;
	    path = open_path;
	    index = 0;
	} else if (time_info.hours == 20) {
	    move = TRUE;
	    path = close_path;
	    index = 0;
	}
    }

    if (cmd || !move || (GET_POS(mayor) < POSITION_SLEEPING) ||
	(GET_POS(mayor) == POSITION_FIGHTING))
	return FALSE;

    switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
	do_move(mayor,"",path[index]-'0'+1);
	break;

    case 'W' :
	GET_POS(mayor) = POSITION_STANDING;
	act("$n awakens and groans loudly.",FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'S' :
	GET_POS(mayor) = POSITION_SLEEPING;
	act("$n lies down and instantly falls asleep.",
	    FALSE, mayor, 0, 0, TO_ROOM);
	break;

    case 'a' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'Hello Honey!'",FALSE,mayor,0,0,TO_ROOM);
	act("$n smirks.",FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'b' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'What a view! I must get something done about that dump!'",
	    FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'c' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
	    FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'd' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'Good day, citizens!'", FALSE, mayor, 0,0,TO_ROOM);
	break;

    case 'e' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'I hereby declare the bazaar open!'",
	    FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'E' :
	if (check_soundproof(mayor)) return(FALSE);
	act("$n says 'I hereby declare Sanctuary closed!'",
	    FALSE,mayor,0,0,TO_ROOM);
	break;

    case 'O' :
	do_unlock(mayor, "gate", 0);
	do_open(mayor, "gate", 0);
	break;

    case 'C' :
	do_close(mayor, "gate", 0);
	do_lock(mayor, "gate", 0);
	break;

    case '.' :
	move = FALSE;
	break;

    }

    index++;
    return TRUE;
}

SPECIAL(ninja_master)
{
    char buf[256];
    static const char *n_skills[] = {
	"track",		/* No. 180 */
	"disarm",		/* No. 245 */
	"ride",
	"doorbash",
	"\n",
    };
    int percent=0, number=0;
    int charge, sk_num, mult;
    struct char_data *master = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (!AWAKE(master))
	return(FALSE);

    if (!master->skills) return(FALSE);

    if (check_soundproof(master)) return(FALSE);

    for(; *arg==' '; arg++);	/* ditch spaces */

    if ((cmd==164)||(cmd==170)) {
	/* So far, just track */
	if (!arg || (strlen(arg) == 0)) {
	    sprintf(buf," track:     %s\n\r",
		    how_good(ch->skills[SKILL_HUNT].learned));
	    send_to_char(buf,ch);
	    sprintf(buf," disarm:    %s\n\r",
		    how_good(ch->skills[SKILL_DISARM].learned));
	    send_to_char(buf,ch);
	    sprintf(buf," ride:      %s\n\r",
		    how_good(ch->skills[SKILL_RIDE].learned));
	    send_to_char(buf,ch);
	    if(HasClass(ch, CLASS_WARRIOR) ||
	       HasClass(ch, CLASS_RANGER) ||
	       HasClass(ch, CLASS_PALADIN))
	    {
		sprintf(buf," doorbash:  %s\n\r",
		    how_good(ch->skills[SKILL_DOORBASH].learned));
		send_to_char(buf,ch);
	    }
	    return(TRUE);
	} else {
	    number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
	    send_to_char ("The ninja master says ",ch);
	    if(IS_NPC(ch)) {
		send_to_char("Perhaps you should return to your original body first.\n\r",ch);

		return(TRUE);
	    }
	    if (number < 1) {
		send_to_char("'I do not know of this skill.'\n\r", ch);
		return(FALSE);
	    }
	    charge = GetMaxLevel(ch) * 100;
	    switch(number) {
	    case 0:
	    case 1:
		sk_num = SKILL_HUNT;
		break;
	    case 2:
		sk_num = SKILL_DISARM;
		mult = 1;
		if ((HasClass(ch, CLASS_MAGIC_USER)) ||
		    (HasClass(ch, CLASS_PSI)) || (HasClass(ch, CLASS_DRUID)))
		    mult = 4;
		if ((HasClass(ch, CLASS_CLERIC)) ||
		    (HasClass(ch, CLASS_PALADIN)))
		    mult = 3;
		if (HasClass(ch, CLASS_THIEF))
		    mult = 2;
		if (HasClass(ch, CLASS_WARRIOR))
		    mult = 1;
		charge *=mult;

		break;
	    case 3:
		sk_num = SKILL_RIDE;
		break;
	    case 4:
		sk_num = SKILL_DOORBASH;
		if((!HasClass(ch, CLASS_WARRIOR)) && (!HasClass(ch, CLASS_RANGER)) &&
		   (!HasClass(ch, CLASS_PALADIN))) {
		    send_to_char("'You do not have the necessary class to practice this skill.'\n\r", ch);
		    return(TRUE);
		}
		break;
	    default:
		sprintf(buf, "Strangeness in ninjamaster (%d)", number);
		log_msg(buf, LOG_MPROG);
		return FALSE;
	    }
	}

	if (GET_GOLD(ch) < charge){
	    send_to_char
		("'Ah, but you do not have enough money to pay.'\n\r",ch);
	    return(TRUE);
	}

	if (ch->skills[sk_num].learned >= 90) {
	    send_to_char
		("'You are a master of this art, I can teach you no more.'\n\r",ch);
	    return(TRUE);
	}

	if((sk_num==SKILL_RIDE || sk_num==SKILL_DOORBASH)
	   && (ch->skills[sk_num].learned >= 45)) {
	    send_to_char
		("'I can teach you no more, you must use this skill to learn it further.'\n\r", ch);
	    return(TRUE);
	}

	if (ch->specials.spells_to_learn <= 0) {
	    send_to_char
		("'You must first use the knowledge you already have.\n\r",ch);
	    return(TRUE);
	}

	GET_GOLD(ch) -= charge;
	send_to_char("'We will now begin.'\n\r",ch);
	ch->specials.spells_to_learn--;

	percent = ch->skills[sk_num].learned +
	    int_app[GET_INT(ch)].learn;
	ch->skills[sk_num].learned = MIN(90, percent);

	if (ch->skills[sk_num].learned >= 90) {
	    send_to_char("'You are now a master of this art.'\n\r", ch);
	    return(TRUE);
	}
    } else {
	return(FALSE);
    }
    return TRUE;
}


SPECIAL(PaladinGuildGuard)
{
    struct char_data *guard = (struct char_data *) me;

    if (type == SPEC_CMD)
	return (FALSE);

    if (type == SPEC_IDLE && AWAKE(guard))
	return attack_convict(guard);

    return generic_warrior(me, ch, cmd, arg, type);
}


SPECIAL(AbyssGateKeeper)
{
    struct char_data *keeper = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	if ((cmd == 1 || cmd == 6) && (!IS_IMMORTAL(ch)) && AWAKE(keeper))
	{
	    if ((cmd == 6) || (cmd == 1))
	    {
		send_to_char
		  ("The gatekeeper shakes his head, and blocks your way.\n\r",
		   ch);
		act("$N shakes $S head, and blocks $n's way.",
		    TRUE, ch, 0, keeper, TO_ROOM);
		return (TRUE);
	    }
	}
	return (FALSE);

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return(FALSE);
}


SPECIAL(RepairGuy)
{
    char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int cost, ave, num;
    struct char_data *vict;
    struct char_data *guy = (struct char_data *) me;
    struct obj_data *obj, *list;

    if (type == SPEC_CMD)
    {
	if (!AWAKE(guy))
	    return(FALSE);

	if (cmd == 72)  /* give */
	{
	    /* determine the correct obj */
	    arg=one_argument(arg,obj_name);
	    if (!*obj_name) {
		send_to_char("Give what?\n\r",ch);
		return(FALSE);
	    }
	    arg=one_argument(arg, vict_name);
	    if(!*vict_name)	{
		send_to_char("To who?\n\r",ch);
		return(FALSE);
	    }
	    if (!(vict = get_char_room_vis(ch, vict_name))) {
		send_to_char("To who?\n\r",ch);
		return(FALSE);
	    }
	    /* the target is the repairman, or an NPC */
	    if (vict != guy)
		return(FALSE);

	    if(getall(obj_name, buf))
	    {
		num = -1;
		strcpy(obj_name, buf);
	    }
	    else if((num = getabunch(obj_name, buf)) != 0)
	    {
		strcpy(obj_name, buf);
	    }
	    else
		num = 1;

	    list = ch->carrying;

	    while(num != 0)
	    {
		if (!(obj = get_obj_in_list_vis(ch, obj_name, list))) {
		    if(num > 0)
			send_to_char("You don't have that item.\n\r",ch);
		    num = 0;
		    break;
		}

		/* we have the repair guy, and we can give him the stuff */
		act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
		act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
		list = obj->next_content;

		/* make all the correct tests to make */
		/* sure that everything is kosher */

		if (ITEM_TYPE(obj) == ITEM_ARMOR &&
		    obj->obj_flags.value[1] > 0)
		{
		    if (obj->obj_flags.value[1] > obj->obj_flags.value[0])
		    {
			/* get the value of the object */
			cost = obj->obj_flags.cost;
			/* divide by value[1]   */
			cost /= obj->obj_flags.value[1];
			/* then cost = difference between value[0] and [1] */
			cost *= (obj->obj_flags.value[1] -
				 obj->obj_flags.value[0]);

			if (GetMaxLevel(vict) > 25) /* super repair guy */
			    cost *= 2;
			if (cost > GET_GOLD(ch)) {
			    if (check_soundproof(ch)) {
				act("$N shakes $S head.\n\r",
				    TRUE, ch, 0, vict, TO_CHAR);
			    } else {
				act("$N says 'I'm sorry, you don't have enough money.'",
				    TRUE, ch, 0, vict, TO_CHAR);
			    }
			} else {
			    GET_GOLD(ch) -= cost;

			    sprintf(buf, "You give $N %d coins.",cost);
			    act(buf,TRUE,ch,0,vict,TO_CHAR);
			    act("$n gives some money to $N.",
				TRUE, ch, obj, vict, TO_ROOM);

			    /* fix the armor */
			    act("$N fiddles with $p.",
				TRUE, ch, obj, vict, TO_CHAR);
			    if (GetMaxLevel(vict) > 25)
				obj->obj_flags.value[0] =
				    obj->obj_flags.value[1];
			    else
			    {
				ave = MAX(obj->obj_flags.value[0],
					  (obj->obj_flags.value[0] +
					   obj->obj_flags.value[1] ) /2);
				obj->obj_flags.value[0] = ave;
				obj->obj_flags.value[1] = ave;
			    }
			    if (check_soundproof(ch))
				act("$N smiles broadly.",
				    TRUE, ch, 0, vict, TO_CHAR);
			    else
			    {
				act("$N says 'All fixed.'",
				    TRUE, ch, 0, vict, TO_ROOM);
				act("$N says 'All fixed.'",
				    TRUE, ch, 0, vict, TO_CHAR);
			    }
			}
		    }
		    else
		    {
			if (check_soundproof(ch))
			{
			    act("$N shrugs.",
				TRUE,ch,0,vict,TO_ROOM);
			    act("$N shrugs.",
				TRUE,ch,0,vict,TO_CHAR);
			}
			else
			{
			    act("$N says 'Your armor looks fine to me.'",
				TRUE,ch,0,vict,TO_ROOM);
			    act("$N says 'Your armor looks fine to me.'",
				TRUE,ch,0,vict,TO_CHAR);
			}
		    }
		}
		else
		{
		    if (check_soundproof(ch))
		    {
			act("$N shakes $S head.\n\r",
			    TRUE, ch, 0, vict, TO_ROOM);
			act("$N shakes $S head.\n\r",
			    TRUE, ch, 0, vict, TO_CHAR);
		    }
		    else
		    {
			if (ITEM_TYPE(obj) != ITEM_ARMOR)
			    act("$N says 'That isn't armor.'",
				TRUE, ch, 0, vict, TO_CHAR);
			else
			{
			    act("$N says 'I can't fix that...'",
				TRUE, ch, 0, vict, TO_CHAR);
			    act("$N says 'I can't fix that...'",
				TRUE, ch, 0, vict, TO_ROOM);
			}
		    }
		}

		act("$N gives you $p.",
		    TRUE,ch,obj,vict,TO_CHAR);

		if(num > 0)
		    num--;
	    }
	    return TRUE;
	}
    }

    else  /* not SPEC_CMD */
	return (generic_warrior(me, ch, cmd, arg, type));

    return (FALSE);
}


SPECIAL(WeaponGuy)
{
    char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int cost, ave, num;
    struct char_data *vict;
    struct char_data *guy = (struct char_data *) me;
    struct obj_data *obj, *list, *nw;

    if (type == SPEC_CMD)
    {
	if (!AWAKE(guy))
	    return(FALSE);

	if (cmd == 72)		/* give */
	{
	    /* determine the correct obj */
	    arg = one_argument(arg,obj_name);
	    if (!*obj_name)
	    {
		send_to_char("Give what?\n\r",ch);
		return(FALSE);
	    }
	    arg=one_argument(arg, vict_name);
	    if(!*vict_name)
	    {
		send_to_char("To who?\n\r",ch);
		return(FALSE);
	    }
	    if (!(vict = get_char_room_vis(ch, vict_name)))
	    {
		send_to_char("To who?\n\r",ch);
		return(FALSE);
	    }

	    /* the target is the repairman, or an NPC */
	    if (vict != guy)
		return(FALSE);

	    if(getall(obj_name, buf))
	    {
		num = -1;
		strcpy(obj_name, buf);
	    }
	    else if((num = getabunch(obj_name, buf)) != 0)
	    {
		strcpy(obj_name, buf);
	    }
	    else
	    {
		num = 1;
	    }

	    list = ch->carrying;

	    while(num != 0)
	    {
		if (!(obj = get_obj_in_list_vis(ch, obj_name, list)))
		{
		    if(num > 0)
			send_to_char("You don't have that item.\n\r",ch);
		    num = 0;
		    break;
		}

		/* we have the repair guy, and we can give him the stuff */
		act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
		act("$n gives $p to $N.",TRUE,ch,obj,vict,TO_ROOM);

		act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
		act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

		list = obj->next_content;

		/* make all the correct tests to make */
		/* sure that everything is kosher */

		if (ITEM_TYPE(obj) == ITEM_WEAPON)
		{
		    nw = make_object(obj->item_number, REAL);
		    if(!nw)
		    {
			if(check_soundproof(ch))
			{
			    act("$N shakes $S head.\n\r",
				TRUE, ch, 0, vict, TO_ROOM);
			    act("$N shakes $S head.\n\r",
				TRUE, ch, 0, vict, TO_CHAR);
			}
			else
			    do_say(ch, "I don't know what that is.", 17);
			goto done;
		    }

		    if(obj->obj_flags.value[2] == nw->obj_flags.value[2])
		    {
			if(check_soundproof(ch))
			{
			    act("$N chuckles slightly.\n\r",
				TRUE, ch, 0, vict, TO_ROOM);
			    act("$N chuckles slightly.\n\r",
				TRUE, ch, 0, vict, TO_CHAR);
			}
			else
			    do_say(ch, "That doesn't need repair.", 17);
			goto done;
		    }

		    cost = SCALE_WEAPON_COST(obj->obj_flags.cost) *
			(nw->obj_flags.value[2] - obj->obj_flags.value[2]) /
			nw->obj_flags.value[2];

		    if (GetMaxLevel(vict) > 25) /* super repair guy */
			cost *= 2;

		    if (cost > GET_GOLD(ch))
		    {
			if (check_soundproof(ch))
			{
			    act("$N shakes $S head.\n\r",
				TRUE, ch, 0, vict, TO_ROOM);
			    act("$N shakes $S head.\n\r",
				TRUE, ch, 0, vict, TO_CHAR);
			}
			else
			    do_say(vict,
				"I'm sorry, you don't have enough money.", 19);
			goto done;
		    }

		    GET_GOLD(ch) -= cost;

		    sprintf(buf, "You give $N %d coins.",cost);
		    act(buf,TRUE,ch,0,vict,TO_CHAR);
		    sprintf(buf, "$N gives you %d coins.", cost);
		    act(buf, TRUE, vict, 0, ch, TO_CHAR);
		    act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);

		    /* fix the weapon */
		    act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);
		    if (GetMaxLevel(vict) > 25)
			obj->obj_flags.value[2] = nw->obj_flags.value[2];
		    else
		    {
			ave = MAX(nw->obj_flags.value[2],
				  (obj->obj_flags.value[2] +
				   nw->obj_flags.value[2]) / 2);
			obj->obj_flags.value[2] = ave;
		    }

		    if (check_soundproof(ch)) {
			act("$N smiles broadly.",TRUE,ch,0,vict,TO_CHAR);
		    }
		    else
			do_say(vict, "All fixed.", 19);

		done:
		    extract_obj(nw);
		}
		else if(check_soundproof(vict))
		{
		    act("$N chuckles.", TRUE, ch, 0, vict, TO_ROOM);
		    act("$N chuckles.", TRUE, ch, 0, vict, TO_CHAR);
		}
		else
		    do_say(vict, "That isn't a weapon.", 19);

		act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
		act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);

		if(num > 0)
		    num --;
	    }
	    return TRUE;
	}
    }

    else   /* not SPEC_CMD */
	return(generic_warrior(me, ch, cmd, arg, type));

    return (FALSE);
}


SPECIAL(MidgaardCitizen)
{
    struct char_data *citizen = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_FIGHT:
	if (check_soundproof(citizen)) return(FALSE);

	if (number(0,18) == 0)
	    do_comm(citizen, "Guards! Help me! Please!", CMD_HOLLER);
	else
	    act("$n shouts 'Guards!  Help me! Please!'",
		TRUE, citizen, 0, 0, TO_ROOM);

	CallForGuard(citizen, citizen->specials.fighting, 3, MIDGAARD);

	/* fall through */

    default:
	generic_warrior(me, ch, cmd, arg, type);
	return(TRUE);

    }

    return(FALSE);
}


SPECIAL(geyser)
{
    struct char_data *geyser = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(geyser))
	return(FALSE);

    if (number(0,3)==0) {
	act("You erupt.", 1, geyser, 0, 0, TO_CHAR);
	cast_geyser(GetMaxLevel(geyser), geyser, "", SPELL_TYPE_SPELL, 0, 0);

	return(TRUE);
    }

    return FALSE;
}


SPECIAL(green_slime)
{
    struct char_data *cons;
    struct char_data *slime = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    int found = FALSE;

    if (cmd || !AWAKE(slime))
	return(FALSE);

    for (cons = real_roomp(slime->in_room)->people; cons;
	 cons = cons->next_in_room )
	if(!IS_NPC(cons) && !IS_IMMORTAL(cons))
	{
	    send_to_char("You've been slimed!", cons);
	    cast_acid_blast(GetMaxLevel(slime), slime, "", SPELL_TYPE_SPELL,
			    cons, 0);
	    found = TRUE;
	}

    return found;
}


SPECIAL(thief)
{
    struct char_data *cons;
    struct char_data *thief = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(thief))
	return(FALSE);

    if (GET_POS(thief) != POSITION_STANDING)
	return FALSE;

    for(cons = real_roomp(thief->in_room)->people; cons;
	cons = cons->next_in_room )
	if(!IS_NPC(cons) && !IS_GOD(cons) && (number(1,5)==1))
	{
	    npc_steal(thief, cons);
	    return TRUE;
	}

    return FALSE;
}


SPECIAL(fighter)
{
    struct char_data *fighter = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(fighter))
	return(FALSE);

    if (fighter->specials.fighting) {
	if (GET_POS(fighter) == POSITION_FIGHTING) {
	    FighterMove(fighter);
	    return TRUE;
	} else {
	    StandUp(fighter);
	    return TRUE;
	}
    }
    return FALSE;
}


SPECIAL(Summoner)
{
    struct char_data *c;
    struct char_data *targ=0;
    struct char_list *i;
    struct char_data *summoner = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(summoner))
	return(FALSE);

    if (check_soundproof(summoner)) return(FALSE);

    if (summoner->specials.fighting)  return(FALSE);

    /*
     **  wait till at 75% of hitpoints.
     */

    if (GET_HIT(summoner) > ((GET_MAX_HIT(summoner)*3)/4)) {
	/*
	 **  check for hatreds
	 */
	if (IS_SET(summoner->hatefield, HATE_CHAR)) {
	    if (summoner->hates.clist) {
		for (i = summoner->hates.clist; i; i = i->next) {
		    if (i->op_ch) { /* if there is a char_ptr */
			targ = i->op_ch;
			if (IS_PC(targ))
			    break;
		    } else {	/* look up the char_ptr */
			EACH_CHARACTER(c_iter, c)
			{
			    if (c && isname(GET_NAME(c), i->name)) {
				targ = c;
				break;
			    }
			}
			END_AITER(c_iter);
		    }
		}
	    }
	}
	if (targ) {
	    act("$n utters the words 'Your ass is mine!'.",
		1, summoner, 0, 0, TO_ROOM);
	    spell_astral_walk(GetMaxLevel(summoner), summoner,
			      SPELL_TYPE_SPELL,targ, 0);
	    if (targ->in_room == summoner->in_room) {
		hit(summoner, targ, 0);
	    }
	    return TRUE;
	}
    }

    return FALSE;
}


/*
New improved magic_user
*/


SPECIAL(magic_user2)
{
    struct char_data *vict;
    struct char_data *mage = (struct char_data *) me;
    ubyte lspell;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(mage))
	return(FALSE);

    if (!mage->specials.fighting) return FALSE;

    if ((GET_POS(mage) > POSITION_STUNNED) &&
	(GET_POS(mage) < POSITION_FIGHTING))
    {
	StandUp(mage);
	return(TRUE);
    }

    vict = FindVictim(mage);

    if (!vict) return(FALSE);

    lspell = number(0,GetMaxLevel(mage)); /* gen number from 0 to level */

    if (lspell < 1) lspell = 1;

    if ((vict!=mage->specials.fighting) && (lspell>19) &&
	(!IS_AFFECTED(mage, AFF_FIRESHIELD)))
    {
	act("$n utters the words 'Fireshield'.", 1, mage, 0, 0, TO_ROOM);
	cast_fireshield(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, mage, 0);
	return TRUE;
    }

    switch (lspell)
    {
    case 1:
	act("$n utters the words 'Chill Touch'.", 1, mage, 0, 0, TO_ROOM);
	cast_chill_touch(GetMaxLevel(mage), mage, "",
			 SPELL_TYPE_SPELL, vict, 0);
	break;
    case 2:
	act("$n utters the words 'Shocking Grasp'.", 1, mage, 0, 0, TO_ROOM);
	cast_shocking_grasp(GetMaxLevel(mage), mage, "",
			    SPELL_TYPE_SPELL, vict, 0);
	break;
    case 3:
    case 4:
	act("$n utters the words 'Frost Cloud'.", 1, mage, 0, 0, TO_ROOM);
	cast_frost_cloud(GetMaxLevel(mage), mage, "",
			 SPELL_TYPE_SPELL, vict, 0);
	break;
    case 5:
	act("$n utters the words 'Burning Hands'.", 1, mage, 0, 0, TO_ROOM);
	cast_burning_hands(GetMaxLevel(mage), mage, "",
			   SPELL_TYPE_SPELL, vict, 0);
	break;
    case 6:
	if (!IS_AFFECTED(vict, AFF_SANCTUARY))
	{
	    act("$n utters the words 'Dispel Magic'.", 1, mage, 0, 0, TO_ROOM);
	    cast_dispel_magic(GetMaxLevel(mage), mage, "",
			      SPELL_TYPE_SPELL, vict, 0);
	}
	else
	{
	    act("$n utters the words 'Acid Blast'.", 1, mage, 0, 0, TO_ROOM);
	    cast_acid_blast(GetMaxLevel(mage), mage, "",
			    SPELL_TYPE_SPELL, vict, 0);
	}
	break;
    case 7:
	act("$n utters the words 'electrocute'.", 1, mage, 0, 0, TO_ROOM);
	cast_electrocute(GetMaxLevel(mage), mage, "",
			 SPELL_TYPE_SPELL, vict, 0);
	break;
    case 8:
	act("$n utters the words 'Fire Wind'.", 1, mage, 0, 0, TO_ROOM);
	cast_fire_wind(GetMaxLevel(mage), mage, "",
		       SPELL_TYPE_SPELL, vict, 0);
	break;
    case 9:
	act("$n utters the words 'Electric Fire'.", 1, mage, 0, 0, TO_ROOM);
	cast_electric_fire(GetMaxLevel(mage), mage, "",
			   SPELL_TYPE_SPELL, vict, 0);
	break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
	act("$n utters the words 'Flamestrike'.", 1, mage, 0, 0, TO_ROOM);
	cast_flamestrike(GetMaxLevel(mage), mage, "",
			 SPELL_TYPE_SPELL, vict, 0);
	break;
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
	act("$n utters the words 'Fireball'.", 1, mage, 0, 0, TO_ROOM);
	cast_fireball(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	break;
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    default:
	act("$n utters the words 'Lava Storm'.",1,mage,0,0,TO_ROOM);
	cast_lava_storm(GetMaxLevel(mage),mage,"",SPELL_TYPE_SPELL,vict,0);
	break;
    }
    return TRUE;
}


SPECIAL(magic_user)
{
    struct char_data *vict;
    struct char_data *mage = (struct char_data *) me;
    ubyte lspell;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(mage) || IS_AFFECTED(mage, AFF_PARALYSIS))
	return(FALSE);

    if (!mage->specials.fighting) {
	if (GetMaxLevel(mage) < 17)
	    return FALSE;
	else {
	    if (!mage->desc) {
		return(Summoner(me, ch, cmd, arg, type));
	    }
	    return FALSE;
	}
    }

    if ((GET_POS(mage) > POSITION_STUNNED) &&
	(GET_POS(mage) < POSITION_FIGHTING)) {
	StandUp(mage);
	return(TRUE);
    }

    if (check_soundproof(mage)) return(FALSE);

    /* Find a dude to to evil things upon ! */

    vict = FindVictim(mage);

    if (!vict) return(FALSE);

    lspell = number(0,GetMaxLevel(mage)); /* gen number from 0 to level */

    /*
	**  check your own problems:
    */

    if (lspell < 1)
	lspell = 1;

    if (IS_AFFECTED(mage, AFF_BLIND) && (lspell > 10)) {
	act("$n utters the words 'Let me see the light!'.",
	    TRUE, mage, 0, 0, TO_ROOM);
	cast_cure_blind(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, mage, 0);
	return TRUE;
    }

    if (IS_AFFECTED(mage, AFF_BLIND))
	return(FALSE);

    if ((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 6) &&
	(GetMaxLevel(mage) > (GetMaxLevel(vict)))) {
	act("$n utters the words 'Use MagicAway Instant Magic Remover'.",
	    1, mage, 0, 0, TO_ROOM);
	cast_dispel_magic(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	return(FALSE);

    }

    if ((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 6) &&
	(GetMaxLevel(mage) > (GetMaxLevel(vict)))) {
	act("$n utters the words 'Use MagicAway Instant Magic Remover'.",
	    1, mage, 0, 0, TO_ROOM);
	cast_dispel_magic(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	return(FALSE);

    }

    if ((GET_HIT(mage) < (GET_MAX_HIT(mage) / 4)) && (lspell > 10) &&
	(!IS_SET(mage->specials.mob_act, ACT_AGGRESSIVE))) {
	act("$n utters the words 'Whoa! I'm outta here!'",
	    1, mage, 0, 0, TO_ROOM);
	cast_teleport(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, mage, 0);
	return(FALSE);
    }


    if ((GET_HIT(mage) > (GET_MAX_HIT(mage) / 2)) && (number(0,1))) {

	/*
	    **  Non-damaging case:
	*/

	if (((lspell>3) && (lspell<50)) && (number(0,6)==0)) {
	    act("$n utters the words 'Icky Sticky!'.", 1, mage, 0, 0, TO_ROOM);
	    cast_web(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    return TRUE;
	}

	if (((lspell>5) && (lspell<10)) && (number(0,6)==0)) {
	    act("$n utters the words 'You wimp'.", 1, mage, 0, 0, TO_ROOM);
	    cast_weakness(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    return TRUE;
	}

	if (((lspell>5) && (lspell<10)) && (number(0,7)==0)) {
	    act("$n utters the words 'Bippety boppity Boom'.",1,mage,0,0,TO_ROOM);
	    cast_armor(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, mage, 0);
	    return TRUE;
	}

	if (((lspell>12) && (lspell<20)) && (number(0,7)==0))	{
	    act("$n utters the words '&#%^^@%*#'.", 1, mage, 0, 0, TO_ROOM);
	    cast_curse(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    return TRUE;
	}

	if (((lspell>7) && (lspell < 20)) && (number(0,5)==0)) {
	    act("$n utters the words 'yabba dabba do'.", 1, mage, 0, 0, TO_ROOM);
	    cast_blindness(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    return TRUE;
	}

	if (((lspell>10) && (lspell < 40)) && (number(0,5)==0) &&
	    (vict->specials.fighting != mage)) {
	    act("$n utters the words 'You are getting sleepy'.",
		1, mage, 0, 0, TO_ROOM);
	    cast_charm(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    if (IS_AFFECTED(vict, AFF_CHARM)) {
		char buf[200];

		if (!vict->specials.fighting) {
		    sprintf(buf, "%s kill %s",
			    GET_NAME(vict), GET_NAME(mage->specials.fighting));
		    do_order(mage, buf, 0);
		} else {
		    sprintf(buf, "%s remove all", GET_NAME(vict));
		    do_order(mage, buf, 0);
		}
	    }
	}

	/*
	    **  The really nifty case:
	*/
	act("$n utters the words 'Here boy!'.", 1, mage, 0, 0, TO_ROOM);
	cast_monsum(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	do_order(mage, "followers guard on", 0);
	return(TRUE);

    } else {

	/*
	    */

	switch (lspell) {
	case 1:
	    act("$n utters the words 'Rather chilly eh!'.", 1, mage, 0, 0, TO_ROOM);
	    cast_chill_touch(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 2:
	    act("$n utters the words 'ZZZZzzzzzzTTTT'.", 1, mage, 0, 0, TO_ROOM);
	    cast_shocking_grasp(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 3:
	case 4:
	    if (mage->attackers <= 2) {
		act("$n utters the words 'Icky Sticky!'.", 1, mage, 0, 0, TO_ROOM);
		cast_web(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    } else {
		act("$n utters the words 'Fwoosh!'.", 1, mage, 0, 0, TO_ROOM);
		cast_burning_hands(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    }
	case 5:
	case 6:
	    act("$n utters the words 'ZAP!'.", 1, mage, 0, 0, TO_ROOM);
	    cast_electrocute(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 7:
	case 8:
	case 9:
	    if (mage->attackers <= 2) {
		act("$n utters the words 'SPOOGE!'.", 1, mage, 0, 0, TO_ROOM);
		cast_acid_blast(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    } else {
		act("$n utters the words 'Get The Sensation!'.", 1, mage, 0, 0, TO_ROOM);
		cast_ice_storm(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    }
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	    if (mage->attackers <= 2) {
		act("$n utters the words 'Im such a Pyro!'.", 1, mage, 0, 0, TO_ROOM);
		cast_flamestrike(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    } else {
		act("$n utters the words 'Ice Ice Baby!'.", 1, mage, 0, 0, TO_ROOM);
		cast_frost_cloud(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	    }
	case 15:
	case 16:
	case 17:
	    act("$n utters the words 'Hasta la vista, Baby'.", 1, mage,0,0,TO_ROOM);
	    cast_fireball(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 18:
	case 19:
	    if (IS_EVIL(mage))	{
		act("$n utters the words 'slllrrrrrrpppp'.", 1, mage, 0, 0, TO_ROOM);
		cast_energy_drain(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		return TRUE;
	    }
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	    if (mage->attackers <= 2) {
		act("$n utters the words 'ZOT!'.", 1, mage, 0,0,TO_ROOM);
		cast_electric_fire(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict,0);
		break;
	    } else {
		act("$n utters the words 'Hasta La Vista, Baby!'.",1,mage,0,0,TO_ROOM);
		cast_fireball(GetMaxLevel(mage),mage,"",SPELL_TYPE_SPELL,vict,0);
		break;
	    }
	default:
	    if (mage->attackers <= 2) {
		if((!IS_SET(mage->specials.mob_act,ACT_POLYSELF)) ||
		   (number(1,3)==2)) {
		    act("$n utters the words 'disrupt'.", 1, mage,0,0,TO_ROOM);
		    cast_disintegrate(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		}
		break;
	    } else {
		if((!IS_SET(mage->specials.mob_act,ACT_POLYSELF)) ||
		   (number(1,3)==2)) {
		    act("$n utters the words 'Burn hard baby!'.", 1, mage,0,0,TO_ROOM);
		    cast_lava_storm(GetMaxLevel(mage), mage, "", SPELL_TYPE_SPELL, vict, 0);
		}
		break;
	    }
	}
    }
    return TRUE;
}


SPECIAL(generic_cleric)
{
    struct char_data *vict;
    struct char_data *cleric = (struct char_data *) me;
    ubyte lspell, healperc=0;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(cleric))
	return(FALSE);

    if (GET_POS(cleric)!=POSITION_FIGHTING) {
	if ((GET_POS(cleric)<POSITION_STANDING) && (GET_POS(cleric)>POSITION_STUNNED)) {
	    StandUp(cleric);
	    return TRUE;
	}
	return FALSE;
    }

    if (check_soundproof(cleric)) return(FALSE);

    if (!cleric->specials.fighting) return FALSE;


    /* Find a dude to to evil things upon ! */

    if ((vict = FindAHatee(cleric))==NULL)
	vict = FindVictim(cleric);

    if (!vict) return(FALSE);

    /*
      gen number from 0 to level
      */

    lspell = number(0,GetMaxLevel(cleric));

    if (lspell < 1)
	lspell = 1;

    /*
      first -- hit a foe, or help yourself?
      */

    if (cleric->points.hit < (cleric->points.max_hit / 2))
	healperc = 7;
    else if (cleric->points.hit < (cleric->points.max_hit / 4))
	healperc = 5;
    else if (cleric->points.hit < (cleric->points.max_hit / 8))
	healperc=3;

    if (number(1,GetMaxLevel(cleric))) {
	/* call lightning */
	if (OUTSIDE(cleric) && (weather_info.sky>=SKY_RAINING) && (lspell >= 15) &&
	    (number(0,5)==0)) {
	    act("$n whistles.",1,cleric,0,0,TO_ROOM);
	    act("$n utters the words 'Here Lightning!'.",1,cleric,0,0,TO_ROOM);
	    cast_call_lightning(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    return(TRUE);
	}

	switch(lspell) {
	case 1:
	    act("$n utters the words 'Getting Hot in Here!'.", 1, cleric,0,0,TO_ROOM);
	    cast_flamestrike(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 2:
	    act("$n utters the words 'Dispel Magic'.", 1, cleric, 0, 0, TO_ROOM);
	    cast_dispel_magic(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 3:
	    act("$n utters the words 'Moo ha ha!'.",1,cleric,0,0,TO_ROOM);
	    cast_harmful_touch(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 4:
	    act("$n utters the words 'What a Rush!'.", 1, cleric,0,0,TO_ROOM);
	    cast_heal(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, cleric, 0);
	    break;
	case 5:
	    act("$n utters the words 'Dispel Magic!'.", 1, cleric,0,0,TO_ROOM);
	    cast_dispel_magic(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 6:
	    act("$n utters the words 'Getting Cold in Here!'.", 1, cleric,0,0,TO_ROOM);
	    cast_frost_cloud(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 7:
	    act("$n utters the words 'Freeze!'.", 1, cleric,0,0,TO_ROOM);
	    cast_ice_storm(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 8:
	    act("$n utters the words 'Rot!'.", 1, cleric,0,0,TO_ROOM);
	    cast_decay(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 9:
	    act("$n utters the words 'Erupt!'.", 1, cleric,0,0,TO_ROOM);
	    cast_rupture(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 10:
	    act("$n utters the words 'Crush!'.", 1, cleric,0,0,TO_ROOM);
	    cast_implode(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	case 11:
	    act("$n utters the words 'What a Rush!'.", 1, cleric,0,0,TO_ROOM);
	    cast_heal(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, cleric, 0);
	    break;
        case 12:
	    act("$n utters the words 'Help me Gods!'.", 1, cleric,0,0,TO_ROOM);
	    cast_sanctuary(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, cleric, 0);
	    break;
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	    act("$n utters the words 'What a Rush!'.", 1, cleric,0,0,TO_ROOM);
	    cast_heal(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, cleric, 0);
	    break;
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	    act("$n utters the words 'Hurts, doesn't it?'.", 1, cleric,0,0,TO_ROOM);
	    cast_implode(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;
	default:
	    act("$n utters the words 'disrupt'.", 1, cleric,0,0,TO_ROOM);
	    cast_disintegrate(GetMaxLevel(cleric), cleric, "", SPELL_TYPE_SPELL, vict, 0);
	    break;

	}
    }
    return TRUE;
}


SPECIAL(guild_guard)
{
    struct char_data *guard = (struct char_data *) me;

    if (type == SPEC_CMD)
    {
   	switch(guard->in_room)
	{
	case 3017:
	    return(CheckForBlockedMove(guard, cmd, arg, 3017, 0,
				       CLASS_MAGIC_USER));
	    break;
	case 3004:
	    return(CheckForBlockedMove(guard, cmd, arg, 3004, 0,
				       CLASS_CLERIC));
	    break;
	case 3027:
	    return(CheckForBlockedMove(guard, cmd, arg, 3027, 1, CLASS_THIEF));
	    break;
	case 3021:
	    return(CheckForBlockedMove(guard, cmd, arg, 3021, 0,
				       CLASS_WARRIOR));
	    break;
	case 2617:
	    return(CheckForBlockedMove(guard, cmd, arg, 2617, 4,
				       CLASS_PALADIN));
	    break;
	case 2619:
	    return(CheckForBlockedMove(guard, cmd, arg, 2619, 0,
				       CLASS_RANGER));
	    break;
	case 2621:
	    return(CheckForBlockedMove(guard, cmd, arg, 2621, 4, CLASS_DRUID));
	    break;
	case 2623:
	    return(CheckForBlockedMove(guard, cmd, arg, 2623, 4, CLASS_PSI));
	    break;
	}
    }
    else
	return(generic_warrior(me, ch, cmd, arg, type));

    return (FALSE);
}

#define INQ_SHOUT 1
#define INQ_LOOSE 0
SPECIAL(Inquisitor)
{
    struct char_data *inquisitor = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_IDLE:
	if (!AWAKE(inquisitor))
	    return(FALSE);

	switch(inquisitor->act_ptr)
	{
	case INQ_SHOUT:
	    if (!check_soundproof(inquisitor))
		do_shout(inquisitor, "Loud and clear, but ya gotta do something about that dragon breath!", 0);
	    inquisitor->act_ptr = 0;
	    return TRUE;
	    break;

	default:
	    break;
	}

    default:
	return(generic_warrior(me, ch, cmd, arg, type));
    }

    return FALSE;
}


SPECIAL(regenerator)
{
    struct char_data *regenerator = (struct char_data *) me;
    int moder, max, cur;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd)
        return(FALSE);

    max = GET_MAX_HIT(regenerator);
    cur = GET_HIT(regenerator);

    if(cur < max)
    {
	moder = max / cur;	/* gain is prop to damage */

	if(moder > (max / 5))	/* limit regen to 20% of total */
	    moder = max / 5;

	if(moder < 5)		/* always regen at least 5 */
	    moder = 5;

	cur += moder;		/* do the regen */

	if(cur > max)		/* check fully healed */
	    cur = max;

	GET_HIT(regenerator) = cur;	/* update real variable */

	update_pos(regenerator);		/* undo stunning, etc. */

        send_to_char("You regenerate.\n\r", regenerator);
	act("$n regenerates.", TRUE, regenerator, 0, 0, TO_ROOM);
	return(TRUE);
    }

    return FALSE;
}

SPECIAL(replicant)
{
    struct char_data *replicant = (struct char_data *) me;
    struct char_data *mob;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return FALSE;

    if (GET_HIT(replicant) < GET_MAX_HIT(replicant))
    {
	/* only 25 replicants in the game at one time! */
	if(mob_index[replicant->nr].number < 25)
	{
	    act("Drops of $n's blood hit the ground, and spring up into another one!",
		TRUE, replicant, 0, 0, TO_ROOM);
	    mob = make_mobile(replicant->nr, REAL);
	    char_to_room(mob, replicant->in_room);
	    act("Two undamaged opponents face you now.", TRUE, replicant, 0, 0, TO_ROOM);
	    GET_HIT(replicant) = GET_MAX_HIT(replicant);

	    return TRUE;
	}
    }

    return FALSE;

}

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4

SPECIAL(Tytan)
{
    struct char_data *tytan = (struct char_data *) me;
    struct char_data *vict;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(tytan))
	return(FALSE);

    if (tytan->specials.fighting) {
	return(magic_user(me, ch, cmd, arg, type));
    } else {
	switch(tytan->act_ptr) {
	case TYT_NONE:
	    if ((vict = FindVictim(tytan))) {
		tytan->act_ptr = TYT_CIT;
		SetHunting(tytan, vict);
	    }
	    break;
	case TYT_CIT:
	    if (tytan->hunt_info && tytan->hunt_info->victim) {
		if (IS_SET(tytan->specials.mob_act, ACT_AGGRESSIVE)) {
		    REMOVE_BIT(tytan->specials.mob_act, ACT_AGGRESSIVE);
		}
		if (tytan->in_room == tytan->hunt_info->victim->in_room) {
		    act("Where is the Citadel?", TRUE, tytan, 0, 0, TO_ROOM);
		    tytan->act_ptr = TYT_WHAT;
		}
	    } else {
		tytan->act_ptr = TYT_NONE;
	    }
	    break;
	case TYT_WHAT:
	    if (tytan->hunt_info && tytan->hunt_info->victim) {
		if (tytan->in_room == tytan->hunt_info->victim->in_room) {
		    act("What must we do?", TRUE, tytan, 0, 0, TO_ROOM);
		    tytan->act_ptr = TYT_TELL;
		}
	    } else {
		tytan->act_ptr = TYT_NONE;
	    }
	    break;
	case TYT_TELL:
	    if (tytan->hunt_info && tytan->hunt_info->victim) {
		if (tytan->in_room == tytan->hunt_info->victim->in_room) {
		    act("Tell Us!  Command Us!", TRUE, tytan, 0, 0, TO_ROOM);
		    tytan->act_ptr = TYT_HIT;
		}
	    } else {
		tytan->act_ptr = TYT_NONE;
	    }
	    break;
	case TYT_HIT:
	    if(tytan->hunt_info && tytan->hunt_info->victim)
	    {
		if (tytan->in_room == tytan->hunt_info->victim->in_room) {
		    if (!check_peaceful(tytan, "The Tytan screams in anger.\n\r")) {
			hit(tytan, tytan->hunt_info->victim, TYPE_UNDEFINED);
			if (!IS_SET(tytan->specials.mob_act, ACT_AGGRESSIVE)) {
			    SET_BIT(tytan->specials.mob_act, ACT_AGGRESSIVE);
			}
			tytan->act_ptr = TYT_NONE;
		    } else {
			tytan->act_ptr = TYT_CIT;
		    }
		}
	    } else {
		tytan->act_ptr = TYT_NONE;
	    }
	    break;
	default:
	    tytan->act_ptr = TYT_NONE;
	}
    }

    return TRUE;
}


SPECIAL(fido)
{
    register struct obj_data *i, *temp, *next_obj, *next_r_obj;
    register struct char_data *v, *next;
    register struct room_data *rp;
    struct char_data *fido = (struct char_data *) me;
    char found = FALSE;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(fido))
	return(FALSE);

    if ((rp = real_roomp(fido->in_room)) == 0)
	return(FALSE);

    for (v = rp->people; (v && (!found)); v = next) {
	next = v->next_in_room;
	if ((IS_NPC(v)) && (mob_index[v->nr].virt == VMOB_ZOM_PET) &&
	    CAN_SEE(fido, v)) {	/* is a zombie */
	    if (v->specials.fighting)
		stop_fighting(v);
	    make_corpse(v);
	    extract_char(v);
	    found = TRUE;
	}
    }

    for (i = real_roomp(fido->in_room)->contents; i; i = next_r_obj) {
	next_r_obj = i->next_content;
	if (IS_CORPSE(i)) {
	    act("$n savagely devours a corpse.", FALSE, fido, 0, 0, TO_ROOM);
	    if(IS_PC_CORPSE(i)) remove("corpsedata.dat");
	    for(temp = i->contains; temp; temp=next_obj)	{
		next_obj = temp->next_content;
		obj_from_obj(temp);
		obj_to_room(temp,fido->in_room);
	    }
	    extract_obj(i);
	    return(TRUE);
	}
    }
    return(FALSE);
}


SPECIAL(janitor)
{
    struct obj_data *i;
    struct char_data *janitor = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(janitor))
        return(FALSE);

    for (i = real_roomp(janitor->in_room)->contents; i; i = i->next_content) {
        if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
            ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
             (i->obj_flags.cost <= 10))) {
            act("$n picks up some trash.", FALSE, janitor, 0, 0, TO_ROOM);

            obj_from_room(i);
            obj_to_char(i, janitor);
            if((!CAN_WEAR(i, ITEM_WIELD)) && (can_wear_test(janitor,i)))
		do_wear(janitor,fname(OBJ_NAME(i)),0);
            return(TRUE);
        }
    }
    return(FALSE);
}


SPECIAL(tormentor)
{
    if (type == SPEC_CMD && !IS_PC(ch) && !IS_IMMORTAL(ch))
	return (TRUE);

    return (FALSE);
}


SPECIAL(RustMonster)
{
    struct char_data *vict;
    struct obj_data *t_item;
    struct char_data *rust = (struct char_data *) me;
    int t_pos;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(rust))
	return(FALSE);

    /*
     **   find a victim
     */
    if(!(vict = FindVictim(rust)))
	return FALSE;

    /*
     **   choose an item of armor or a weapon that is metal
     **  since metal isn't defined, we'll just use armor and weapons
     */

    /*
     **  choose a weapon first, then if no weapon, choose a shield,
     **  if no shield, choose breast plate, then leg plate, sleeves,
     **  helm
     */

    if (vict->equipment[WIELD]) {
	t_item = vict->equipment[WIELD];
	t_pos = WIELD;
    } else if (vict->equipment[WEAR_SHIELD]) {
	t_item = vict->equipment[WEAR_SHIELD];
	t_pos = WEAR_SHIELD;
    } else if (vict->equipment[WEAR_BODY]) {
	t_item = vict->equipment[WEAR_BODY];
	t_pos = WEAR_BODY;
    } else if (vict->equipment[WEAR_LEGS]) {
	t_item = vict->equipment[WEAR_LEGS];
	t_pos = WEAR_LEGS;
    } else if (vict->equipment[WEAR_ARMS]) {
	t_item = vict->equipment[WEAR_ARMS];
	t_pos = WEAR_ARMS;
    } else if (vict->equipment[WEAR_HEAD]) {
	t_item = vict->equipment[WEAR_HEAD];
	t_pos = WEAR_HEAD;
    } else {
	return(FALSE);
    }

    /*
     **  item makes save (or not)
     */
    if (DamageOneItem(vict, ACID_DAMAGE, t_item)) {
	t_item = unequip_char(vict, t_pos);
	if (t_item) {
	    /*
	     **  if it doesn't make save, falls into a pile of scraps
	     */
	    MakeScrap(vict, t_item, ACID_DAMAGE);
	}
    }

    return TRUE;
}


SPECIAL(temple_labrynth_liar)
{
    struct char_data *liar = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(liar))
	return(0);

    if (check_soundproof(liar)) return(FALSE);

    switch (number(0, 15)) {
    case 0:
	do_say(liar, "I'd go west if I were you.", 0);
	return(1);
    case 1:
	do_say(liar, "I heard that Vile is a cute babe.", 0);
	return(1);
    case 2:
	do_say(liar, "Going east will avoid the beast!", 0);
	return(1);
    case 4:
	do_say(liar, "North is the way to go.", 0);
	return(1);
    case 6:
	do_say(liar, "Dont dilly dally go south.", 0);
	return(1);
    case 8:
	do_say(liar, "Great treasure lies ahead", 0);
	return(1);
    case 10:
	do_say(liar, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
	return(1);
    case 12:
	do_say(liar, "I am a very clever liar.", 0);
	return(1);
    case 14:
	do_say(liar, "Loki is a really great guy!", 0);
	do_say(liar, "Well.... maybe not...", 0);
	return(1);
    default:
	do_say(liar, "Then again I could be wrong!", 0);
	return(1);
    }
}


SPECIAL(temple_labrynth_sentry)
{
    struct char_data *tch;
    struct char_data *sentry = (struct char_data *) me;
    int counter;

    if (type == SPEC_INIT)
	return (FALSE);

    if(cmd || !AWAKE(sentry)) return FALSE;

    if(GET_POS(sentry)!=POSITION_FIGHTING) return FALSE;

    if(!sentry->specials.fighting) return FALSE;

    if (check_soundproof(sentry)) return(FALSE);

    /* Find a dude to do very evil things upon ! */

    for (tch=real_roomp(sentry->in_room)->people; tch; tch = tch->next_in_room) {
	if( GetMaxLevel(tch)>10 && CAN_SEE(sentry, tch)) {
	    act("The sentry snaps out of his trance and ...", 1, sentry, 0, 0, TO_ROOM);
	    do_say(sentry, "You will die for your insolence, pig-dog!", 0);
	    for ( counter = 0 ; counter < 4 ; counter++ )
		if ( GET_POS(tch) > POSITION_SITTING)
		    cast_fireball(15, sentry, "", SPELL_TYPE_SPELL, tch, 0);
		else
		    return TRUE;
	    return TRUE;
	}
	else
	{
	    act("The sentry looks concerned and continues to push you away",
		1, sentry, 0, 0, TO_ROOM);
	    do_say(sentry, "Leave me alone. My vows do not permit me to kill you!", 0);
	}
    }
    return FALSE;
}


#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

SPECIAL(NudgeNudge)
{
    struct char_data *vict;
    struct char_data *nudge = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(nudge))
	return (FALSE);

    if (nudge->specials.fighting) {
	return(FALSE);
    }


    switch(nudge->act_ptr) {
    case NN_LOOSE:
	/*
	 ** find a victim
	 */
	vict = FindVictim(nudge);
	if (!vict)
	    return(FALSE);
	/* start following */
	if (circle_follow(nudge, vict)) {
	    return(FALSE);
	}
	if (nudge->master)
	    stop_follower(nudge);
	add_follower(nudge, vict, 0);
	nudge->act_ptr = NN_FOLLOW;
	if (!check_soundproof(nudge))
	    do_say (nudge, "Good Evenin' Squire!" , 0 );
	act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	break;
    case NN_FOLLOW:
	switch(number(0,20)) {
	case 0:
	    if (!check_soundproof(nudge))
		do_say  (nudge, "Is your wife a goer?  Know what I mean, eh?", 0 );
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    break;
	case 1:
	    act ("$n winks at you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n winks at you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n winks at $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n winks at $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    if (!check_soundproof(nudge))
		do_say  (nudge, "Say no more!  Say no MORE!", 0);
	    break;
	case 2:
	    if (!check_soundproof(nudge)) {
		do_say  (nudge, "You been around, eh?", 0);
		do_say  (nudge, "...I mean you've ..... done it, eh?", 0);
	    }
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    break;
	case 3:
	    if (!check_soundproof(nudge))
		do_say  (nudge, "A nod's as good as a wink to a blind bat, eh?", 0);
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n nudges you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n nudges $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    break;
	case 4:
	    if (!check_soundproof(nudge))
		do_say  (nudge, "You're WICKED, eh!  WICKED!", 0);
	    act ("$n winks at you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n winks at you.", FALSE, nudge, 0, 0, TO_CHAR);
	    act ("$n winks at $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    act ("$n winks at $N.", FALSE, nudge, 0, nudge->master, TO_ROOM);
	    break;
	case 5:
	    if (!check_soundproof(nudge))
		do_say  (nudge, "Wink. Wink.", 0);
	    break;
	case 6:
	    if (!check_soundproof(nudge))
		do_say  (nudge, "Nudge. Nudge.", 0);
	    break;
	case 7:
	case 8:
	    nudge->act_ptr = NN_STOP;
	    break;
	default:
	    break;
	}
	break;
    case NN_STOP:
	/*
	 **  Stop following
	 */
	if (!check_soundproof(nudge))
	    do_say(nudge, "Evening, Squire", 0);
	stop_follower(nudge);
	nudge->act_ptr = NN_LOOSE;
	break;
    default:
	nudge->act_ptr = NN_LOOSE;
	break;
    }

    return TRUE;
}


SPECIAL(AGGRESSIVE)
{
    struct char_data *self = (struct char_data *) me;
    struct char_data *tch;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(self) || self->specials.fighting)
	return(FALSE);

    for (tch=real_roomp(self->in_room)->people; tch; tch=tch->next_in_room) {
	if (tch!=self && IS_PC(tch) &&
	    !IS_SET(tch->specials.flags, PLR_NOHASSLE) &&
	    CAN_SEE(self, tch) && !IS_AFFECTED(self, AFF_SNEAK)) {
	    hit(self, tch, TYPE_UNDEFINED);
	    return (TRUE);
	}
    }

    return (FALSE);
}


SPECIAL(citizen)
{
    struct char_data *citizen = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_FIGHT:
	if (!check_soundproof(citizen))
	{
	    if (number(0,18) == 0)
		do_comm(citizen, "Guards! Help me! Please!", CMD_HOLLER);
	    else
		act("$n shouts 'Guards!  Help me! Please!'",
		    TRUE, citizen, 0, 0, TO_ROOM);
	}

	/* fall through */

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return(FALSE);
}


SPECIAL(cityguard)
{
    struct char_data *tch, *evil;
    struct char_data *guard = (struct char_data *) me;
    int max_evil;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_IDLE:
	if (!AWAKE(guard))
	    return (FALSE);

	max_evil = 1000;
	evil = 0;

	if (check_peaceful(guard, ""))
	    return FALSE;

	for (tch = real_roomp(guard->in_room)->people; tch;
	     tch = tch->next_in_room)
	{
	    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(guard, tch))
	    {
		max_evil = -1000;
		evil = tch;
		if (!check_soundproof(guard))
		    act("$n screams 'EVIL!!!  BANZAI!  SPOOON!'",
			FALSE, guard, 0, 0, TO_ROOM);
		hit(guard, evil, TYPE_UNDEFINED);
		return(TRUE);
	    }

	    if (IS_SET(tch->specials.flags, PLR_PKILLER))
	    {
		if(!check_soundproof(guard))
		    act("$n screams 'MURDERER!!!'",
			FALSE, guard, 0, 0, TO_ROOM);
		hit(guard, tch, TYPE_UNDEFINED);
		return TRUE;
	    }
	    if (tch->specials.fighting)
	    {
		if ((GET_ALIGNMENT(tch) < max_evil) &&
		    (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
		{
		    max_evil = GET_ALIGNMENT(tch);
		    evil = tch;
		}
	    }
	}

	if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0))
	{
	    if (GET_HIT(evil->specials.fighting) > GET_HIT(guard) ||
		(evil->specials.fighting->attackers > 3))
	    {
		if (!check_soundproof(guard))
		    act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
			FALSE, guard, 0, 0, TO_ROOM);
		hit(guard, evil, TYPE_UNDEFINED);
		return(TRUE);
	    }
	    else
	    {
		if (!check_soundproof(guard))
		    act("$n yells 'There's no need to fear!  A Cityguard is here!'",
			FALSE, guard, 0, 0, TO_ROOM);

		if (!guard->skills)
		    SpaceForSkills(guard);

		if (!guard->skills[SKILL_RESCUE].learned)
		    guard->skills[SKILL_RESCUE].learned = GetMaxLevel(guard)*3+30;
		do_rescue(guard, GET_NAME(evil->specials.fighting), 0);
	    }
	}
	return (FALSE);

    case SPEC_FIGHT:
	guard_help_call(guard, 3, MIDGAARD);
	/* fall through */

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return(FALSE);
}


SPECIAL(Ringwraith)
{
    static char buf[256];
    struct char_data *victim;
    struct char_data *wraith = (struct char_data *) me;
    static int	howmanyrings=-1;
    struct obj_data	*ring;
    struct wraith_hunt {
	int	ringnumber;
	int	chances;
    } *wh;
    room_num rnum;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_IDLE:
	if (!AWAKE(wraith))
	    return (FALSE);

	if (!wraith->act_ptr)
	    STATE0("Ringwraith special: No act_ptr");
	else
	{
	    sprintf(buf, "Ringwraith special: ringnumber %d, chance %d",
		    ((struct wraith_hunt *) wraith->act_ptr)->ringnumber,
		    ((struct wraith_hunt *) wraith->act_ptr)->chances);
	    STATE0(buf);
	}

	if (howmanyrings==-1) 	/* how many one rings are in the game? */
	{
	    howmanyrings = 1;
	    get_obj_vis_world(wraith, "999.one ring", &howmanyrings);
	}

	if (wraith->act_ptr==0) /* does our ringwraith have his state info? */
	{
	    CREATE(wh, struct wraith_hunt, 1);
	    wraith->act_ptr = (int) wh;
	    wh->ringnumber=0;
	}
	else
	    wh = (struct wraith_hunt*)wraith->act_ptr;

	if (!wh->ringnumber) 	/* is he currently tracking a ring */
	{
	    wh->chances = 0;
	    wh->ringnumber = number(1, howmanyrings++);
	}

	/* where is this ring? */
	sprintf(buf, "%d.one ring.", (int)wh->ringnumber);
	if (NULL == (ring = get_obj_vis_world(wraith, buf, NULL)))
	{
	    /* there aren't as many one rings in the game as we thought */
	    howmanyrings = 1;
	    get_obj_vis_world(wraith, "999.one ring", &howmanyrings);
	    wh->ringnumber = 0;
	    return FALSE;
	}

	rnum = room_of_object(ring);

	if (rnum != wraith->in_room)
	{
	    path_kill(wraith->hunt_info);
	    wraith->hunt_info =
		path_to_room(wraith->in_room, rnum, 5000, HUNT_GLOBAL);
	    if(!wraith->hunt_info)
	    {
		wh->ringnumber = 0;
		return FALSE;
	    }
	    go_direction(wraith,
			 path_dir(wraith->in_room, wraith->hunt_info), 0);
	    return TRUE;
	}

	/* the ring is in the same room! */

	if ((victim = char_holding(ring)))
	{
	    if (victim==wraith)
	    {
		obj_from_char(ring);
		extract_obj(ring);
		wh->ringnumber=0;
		act("$n grimaces happily.",
		    FALSE, wraith, NULL, victim, TO_ROOM);
	    }
	    else
	    {
		switch (wh->chances)
		{

		case 0:
		    do_wake(wraith, GET_NAME(victim), 0);
		    if (!check_soundproof(wraith))
			act("$n says '$N, give me The Ring'.",
			    FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));
		    else
			act("$n pokes you in the ribs.",
			    FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));
		    wh->chances++;
		    return(TRUE);

		case 1:
		    if (IS_NPC(victim))
                    {
                        if (!check_soundproof(wraith))
                            act("$n says '$N, give me The Ring *NOW*'.",
                                FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));
                        else
                            act("$n pokes you in the ribs very painfully.",
                                FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));

                        wh->chances++;
                    }
		    else
		    {
			if (!check_soundproof(wraith))
			    act("$n says '$N, give me The Ring *NOW*'.",
				FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));
			else
			    act("$n pokes you in the ribs very painfully.",
				FALSE, wraith, NULL, victim, (TO_ROOM|TO_VICT));

			wh->chances++;
		    }
		    return(TRUE);

		default:
		    if (check_peaceful(wraith,
				       "Damn, he's in a safe spot.\n\r"))
		    {
			if (!check_soundproof(wraith))
			    act("$n says 'You can't stay here forever, $N'.",
				FALSE, wraith, NULL, victim, (TO_VICT|TO_ROOM));
		    }
		    else
		    {
			if (!check_soundproof(wraith))
			    act("$n says 'I guess I'll just have to get it myself'.",
				FALSE, wraith, NULL, victim, (TO_VICT|TO_ROOM));
			hit(wraith, victim, TYPE_UNDEFINED);
		    }
		    break;
		}
	    }
	}
	else if (ring->in_obj)
	{
	    /* the ring is in an object */
	    obj_from_obj(ring);
	    obj_to_char(ring, wraith);
	    act("$n gets the One Ring.", FALSE, wraith, NULL, victim, TO_ROOM);
	    return TRUE;
	}
	else if (ring->in_room != NOWHERE)
	{
	    obj_from_room(ring);
	    obj_to_char(ring, wraith);
	    act("$n gets the Ring.", FALSE, wraith, NULL, 0, TO_ROOM);
	    return TRUE;
	}
	else
	{
	    log_msg("a One Ring was completely disconnected!?", LOG_MPROG);
	    wh->ringnumber = 0;
	}
	return (FALSE);

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return (FALSE);
}


/***********************************************************************

			   CHESSBOARD PROCS

 ***********************************************************************/

/*
 *  list of room #s
 */

#define SISYPHUS_MAX_LEVEL 12

/* This is the highest level of PC that can enter.  The highest level
   monster currently in the section is 14th.  It should require a fairly
   large party to sweep the section. */

SPECIAL(sisyphus)
{
    struct char_data *sisyphus = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return Blocker(me, ch, cmd, arg, type);

    case SPEC_FIGHT:
	if ((GET_POS(sisyphus) < POSITION_FIGHTING) &&
	    (GET_POS(sisyphus) > POSITION_STUNNED))
	    StandUp(sisyphus);
	else
	    FighterMove(sisyphus);

	return TRUE;
    }

    return(FALSE);
}				/* end sisyphus */


SPECIAL(jabberwocky)
{
    struct char_data *wocky = (struct char_data *) me;

    switch (type)
    {
    case SPEC_FIGHT:
	if ((GET_POS(wocky) < POSITION_FIGHTING) &&
	    (GET_POS(wocky) > POSITION_STUNNED))
	    StandUp(wocky);
	else
	    FighterMove(wocky);

	return TRUE;
    }

    return FALSE;
}


SPECIAL(flame)
{
    struct char_data *flame = (struct char_data *) me;

    switch (type)
    {
    case SPEC_FIGHT:
	if ((GET_POS(flame) < POSITION_FIGHTING) &&
	    (GET_POS(flame) > POSITION_STUNNED))
	    StandUp(flame);
	else
	    FighterMove(flame);

	return TRUE;
    }

    return FALSE;
}


SPECIAL(jugglernaut)
{
    struct obj_data *tmp_obj;
    struct char_data *naut = (struct char_data *) me;
    int i, j;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return(FALSE);

    if (GET_POS(naut) == POSITION_STANDING) {

	if (random()%3) return FALSE;

	/* juggle something */

	if (IS_CARRYING_N(naut) < 1) return FALSE;

	i = random()%IS_CARRYING_N(naut);
	j = 0;
	for (tmp_obj = naut->carrying; (tmp_obj) && (j < i); j++) {
	    tmp_obj = tmp_obj->next_content;
	}

	if (random()%6) {
	    if (random()%2) {
		act("$n tosses $p high into the air and catches it.", TRUE, naut, tmp_obj, NULL, TO_ROOM);
	    }  else {
		act("$n sends $p whirling.", TRUE, naut, tmp_obj, NULL, TO_ROOM);
	    }
	} else {
	    act("$n tosses $p but fumbles it!", TRUE, naut, tmp_obj, NULL, TO_ROOM);
	    obj_from_char(tmp_obj);
	    obj_to_room(tmp_obj, naut->in_room);
	}
	return(TRUE);		/* don't move, I dropped something */
    }
    return(FALSE);
}

#if 0
static char *elf_comm[] = {
  "wake", "yawn",
  "stand", "say Well, back to work.", "get all",
  "eat bread", "wink",
  "w", "w", "s", "s", "s", "d", "open gate", "e",  /* home to gate*/
  "close gate",
  "e", "e", "e", "e", "n", "w", "n", /* gate to baker */
  "give all.bread baker", /* pretend to give a bread */
  "give all.pastry baker", /* pretend to give a pastry */
  "say That'll be 33 coins, please.",
  "echo The baker gives some coins to the Elf",
  "wave",
  "s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry",
  "w", "s", "s", /* to main square */
  "s", "w", "w", "w", "w", /* back to gate */
  "pat sisyphus",
  "open gate", "w", "close gate", "u", "n", "n", "n", "e", "e", /* to home */
  "say Whew, I'm exhausted.", "rest", "$"};
#endif

SPECIAL(delivery_elf)
{
#define ELF_INIT     0
#define ELF_RESTING  1
#define ELF_GETTING  2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER   5
#define ELF_RETURN_HOME    6

    struct char_data *elf = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return(FALSE);

  if (elf->specials.fighting)
    return FALSE;

  if(elf->hunt_info)
    return FALSE;

  /* Extra debugging info */
  STATE1("Delivery elf special proc, act_ptr: %d", elf->act_ptr);

  switch(elf->act_ptr) {

  case ELF_INIT:
    if (elf->in_room == 0) {
      /* he has been banished to the Void */
    } else if (elf->in_room != Elf_Home) {
      if (GET_POS(elf) == POSITION_SLEEPING) {
	do_wake(elf, "", 0);
	do_stand(elf, "", 0);
      }
      do_say(elf, "Woah! How did i get here!", 0);
      do_emote(elf, "waves his arm, and vanishes!", 0);
      char_from_room(elf);
      char_to_room(elf, Elf_Home);
      do_emote(elf, "arrives with a Bamf!", 0);
      do_emote(elf, "yawns", 0);
      do_sleep(elf, "", 0);
      elf->act_ptr = ELF_RESTING;
    } else {
      elf->act_ptr = ELF_RESTING;
    }
    return TRUE;
  case ELF_RESTING:
    if ((time_info.hours > 6) && (time_info.hours < 9)) {
      do_wake(elf, "", 0);
      do_stand(elf, "", 0);
      elf->act_ptr = ELF_GETTING;
      return TRUE;
    }
    break;

  case ELF_GETTING:
    do_get(elf, "all.loaf", 0);
    do_get(elf, "all.biscuit", 0);
    elf->act_ptr = ELF_DELIVERY;
    return TRUE;
  case ELF_DELIVERY:
    if (elf->in_room != Bakery) {
      track_to_room(elf, Bakery, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
    } else {
      do_give(elf, "6*biscuit baker", 0);
      do_give(elf, "6*loaf baker", 0);
      do_say(elf, "That'll be 33 coins, please.", 0);
      elf->act_ptr = ELF_DUMP;
      return TRUE;
    }
    break;

  case ELF_DUMP:
    if (elf->in_room != Dump)   {
      track_to_room(elf, Dump, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
    } else {
      do_drop(elf, "10*biscuit", 0);
      do_drop(elf, "10*loaf", 0);
      elf->act_ptr = ELF_RETURN_TOWER;
      return(TRUE);
    }
    break;
  case ELF_RETURN_TOWER:
    if (elf->in_room != Ivory_Gate)   {
      track_to_room(elf, Ivory_Gate, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
    } else {
      elf->act_ptr = ELF_RETURN_HOME;
    }
    break;
  case ELF_RETURN_HOME:
    if (elf->in_room != Elf_Home)   {
      track_to_room(elf, Elf_Home, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
    } else {
      if (time_info.hours > 21) {
	do_say(elf, "Done at last!", 0);
	do_sleep(elf, "", 0);
	elf->act_ptr = ELF_RESTING;
	return(TRUE);
      } else {
	do_say(elf, "An elf's work is never done.", 0);
	elf->act_ptr = ELF_GETTING;
      }
    }
    break;
  default:
    elf->act_ptr = ELF_INIT;
    return(FALSE);
  }
  return FALSE;
}


SPECIAL(delivery_beast)
{
    struct obj_data *o;
    struct char_data *beast = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return(FALSE);

    if (time_info.hours == 6) {
	do_drop(beast, "all.loaf",0);
	do_drop(beast, "all.biscuit", 0);
	return TRUE;
    } else if (time_info.hours < 2) {
	if (number(0,1)) {
	    if((o = make_object(3012, VIRTUAL)))
		obj_to_char(o, beast);
	} else {
	    if((o = make_object(3013, VIRTUAL)))
		obj_to_char(o, beast);
	}
    } else {
	if (GET_POS(beast) > POSITION_SLEEPING) {
	    do_sleep(beast, "", 0);
	    return TRUE;
	}
    }

    return FALSE;
}


/*
 * ABYSS Special Procs
 */
SPECIAL(Keftab)
{
    int found, targ_item;
    struct char_data *i;
    struct char_data *keftab = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return(FALSE);

    if (!keftab->hunt_info) {
	/* find a victim */

	found = FALSE;
	EACH_CHARACTER(iter, i)
	{
	    if (!IS_NPC(keftab))
	    {
		for(targ_item = SWORD_ANCIENTS ;
		    !found && (targ_item <= SWORD_ANCIENTS + 20);
		    targ_item++)
	        {
		    if ((HasObject(i, targ_item))&&(GetMaxLevel(i) < 30))
		    {
			AddHated(keftab, i);
			SetHunting(keftab, i);
			found = TRUE;
		    }
		}
	    }
	}
	END_AITER(iter);

	return(FALSE);
    } else {

	/* check to make sure that the victim still has an item */
	found = FALSE;
	for(targ_item = SWORD_ANCIENTS ;
	    !found && (targ_item <= SWORD_ANCIENTS + 20);
	    targ_item++)
	{
	    if ((HasObject(keftab->hunt_info->victim, targ_item))
		&&(GetMaxLevel(keftab->hunt_info->victim) < 30))
		found = TRUE;
	}
	if(!found)
	{
	    path_kill(keftab->hunt_info);
	    keftab->hunt_info = 0;
	}
    }
    return FALSE;
}


SPECIAL(StormGiant)
{
  struct char_data *vict;
  struct char_data *giant = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(giant))
    return(FALSE);

  if (!(vict=giant->specials.fighting))
    return (FALSE);

  if(GET_POS(giant)<POSITION_FIGHTING) {
    if (IS_PC(giant))
      return (FALSE);
    StandUp(giant);
    return (TRUE);
  }

  if (number(1,5)<4)
    return generic_warrior(me, ch, cmd, arg, type);
  else {
    act("$n creates a crackling ball of electricity!", TRUE, giant, 0, 0,TO_ROOM);
    cast_electrocute(GetMaxLevel(giant), giant, "", SPELL_TYPE_SPELL, vict, 0);
    return (TRUE);
  }

  return FALSE;
}

/*
**  NEW THALOS MOBS:******************************************************
*/

#define NTMWMORN    0
#define NTMSTARTM   1
#define NTMGOALNM   2
#define NTMGOALEM   3
#define NTMGOALSM   4
#define NTMGOALWM   5
#define NTMGOALOM   6
#define NTMWNIGHT   7
#define NTMSTARTN   8
#define NTMGOALNN   9
#define NTMGOALEN   10
#define NTMGOALSN   11
#define NTMGOALWN   12
#define NTMGOALON   13
#define NTMSUSP     14
#define NTM_FIX     15

SPECIAL(NewThalosMayor)
{
    struct char_data *mayor = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

  if (cmd || !AWAKE(mayor))
    return(FALSE);

  if (mayor->specials.fighting)
    return(FALSE);
  else if(!mayor->hunt_info)
  {

    switch(mayor->act_ptr) {	/* state info */
    case NTMWMORN:		/* wait for morning */
      if (time_info.hours == 6) {
	mayor->act_ptr = NTMGOALNM;
      }
      break;
    case NTMGOALNM:		/* north gate */
      if (mayor->in_room != NTMNGATE) {
	track_to_room(mayor, NTMNGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * unlock and open door.
	 */
	do_unlock(mayor, " gate", 0);
	do_open(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALEM;
	return TRUE;
      }
      break;
    case NTMGOALEM:
      if (mayor->in_room != NTMEGATE) {
	track_to_room(mayor, NTMEGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * unlock and open door.
	 */
	do_unlock(mayor, " gate", 0);
	do_open(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALSM;
	return TRUE;
      }
      break;
    case NTMGOALSM:
      if (mayor->in_room != NTMSGATE) {
	track_to_room(mayor, NTMSGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * unlock and open door.
	 */
	do_unlock(mayor, " gate", 0);
	do_open(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALWM;
	return TRUE;
      }
      break;

    case NTMGOALWM:
      if (mayor->in_room != NTMWGATE) {
	track_to_room(mayor, NTMWGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * unlock and open door.
	 */
	do_unlock(mayor, " gate", 0);
	do_open(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALOM;
	return TRUE;
      }
      break;
    case NTMGOALOM:
      if (mayor->in_room != NTMOFFICE) {
	track_to_room(mayor, NTMOFFICE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	mayor->act_ptr = NTMWNIGHT;
      }
      break;

    case NTMWNIGHT:		/* go back to wait for 7pm */
      if (time_info.hours == 19) {
	mayor->act_ptr = NTMGOALNN;
      }
      break;

    case NTMGOALNN:		/* north gate */
      if (mayor->in_room != NTMNGATE) {
	track_to_room(mayor, NTMNGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * lock and open door.
	 */
	do_lock(mayor, " gate", 0);
	do_close(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALEN;
	return TRUE;
      }
      break;

    case NTMGOALEN:
      if (mayor->in_room != NTMEGATE) {
	track_to_room(mayor, NTMEGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * lock and open door.
	 */
	do_lock(mayor, " gate", 0);
	do_close(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALSN;
	return TRUE;
      }
      break;
    case NTMGOALSN:
      if (mayor->in_room != NTMSGATE) {
	track_to_room(mayor, NTMSGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * lock and open door.
	 */
	do_lock(mayor, " gate", 0);
	do_close(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALWN;
	return TRUE;
      }
      break;
    case NTMGOALWN:
      if (mayor->in_room != NTMWGATE) {
	track_to_room(mayor, NTMWGATE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	/*
	 * unlock and open door.
	 */
	do_lock(mayor, " gate", 0);
	do_close(mayor, " gate", 0);
	mayor->act_ptr = NTMGOALOM;
	return TRUE;
      }
      break;
    case NTMGOALON:
      if (mayor->in_room != NTMOFFICE) {
	track_to_room(mayor, NTMOFFICE, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
      } else {
	mayor->act_ptr = NTMWMORN;
      }
      break;
    case NTM_FIX:
      /*
       * move to correct spot (office)
       */
      do_say(mayor, "Woah! How did i get here!", 0);
      char_from_room(mayor);
      char_to_room(mayor, NTMOFFICE);
      mayor->act_ptr = NTMWMORN;
      return TRUE;
    default:
      mayor->act_ptr = NTM_FIX;
      break;
    }
  }
  return FALSE;
}


SPECIAL(SultanGuard)
{
    struct char_data *guard = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	break;

    case SPEC_IDLE:
	if (!AWAKE(guard))
	    break;

	return guard_idle_action(guard);

    case SPEC_FIGHT:
	guard_help_call(guard, 3, NEWTHALOS);
	/* fall through */

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return (FALSE);
}


/******************Mordilnia citizens************************************/

SPECIAL(MordGuard)
{
    struct char_data *guard = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_IDLE:
	if (!AWAKE(guard))
	    return (FALSE);

	return guard_idle_action(guard);

    case SPEC_FIGHT:
	guard_help_call(guard, 3, MORDILNIA);
	/* fall through */

    default:
	return generic_warrior(me, ch, cmd, arg, type);
    }

    return(FALSE);
}


SPECIAL(warrior_blocker)
{
    switch (type)
    {
    case SPEC_CMD:
	return Blocker(me, ch, cmd, arg, type);

    default:
	return(generic_warrior(me, ch, cmd, arg, type));
    }

    return (FALSE);
}


SPECIAL(Devil)
{
    return(magic_user(me, ch, cmd, arg, type));
}


SPECIAL(Demon)
{
    return(magic_user(me, ch, cmd, arg, type));
}


/* gypsy village mobs */

SPECIAL(StatTeller)
{
    struct char_data *teller = (struct char_data *) me;
    int choice;
    char buf[200];

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) {
	if (cmd == 56) {	/* buy */

	    /*
	     ** randomly tells a player 3 of his/her stats.. for a price
	     */
	    if (GET_GOLD(ch)< 10000) {
		send_to_char("You do not have the money to pay me.\n\r", ch);
		return(TRUE);
	    } else {
		GET_GOLD(ch)-=10000;
	    }

	    choice = number(0,2);
	    switch(choice) {
	    case 0:
		sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r", GET_STR(ch), GET_WIS(ch), GET_DEX(ch));
		send_to_char(buf, ch);
		break;
	    case 1:
		sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r", GET_INT(ch), GET_DEX(ch), GET_CON(ch));
		send_to_char(buf, ch);
		break;
	    case 2:
		sprintf(buf, "CON: %d, INT: %d , WIS: %d \n\r", GET_CON(ch), GET_INT(ch), GET_WIS(ch));
		send_to_char(buf, ch);
		break;
	    default:
		send_to_char("We are experiencing Technical difficulties\n\r", ch);
	    }
	    return(TRUE);
	}
    }else {

	/*
	 **  in combat, issues a more potent curse.
	 */

	if (teller->specials.fighting) {
	    act("$n gives you the evil eye!  You feel your hitpoints ebbing away",
		FALSE, teller, 0, teller->specials.fighting, TO_VICT);
	    act("$n gives $N the evil eye!  $N seems weaker!",
		FALSE, teller, 0, teller->specials.fighting, TO_NOTVICT);
	    teller->specials.fighting->points.max_hit -= 10;
	    teller->specials.fighting->points.hit -= 10;
	    return TRUE;
	}

    }
    return(FALSE);
}

/* utility for thrower */
int ThrowChar(struct char_data *ch, struct char_data *v, int dir)
{
    struct room_data *rp;
    int or2;
    char buf[200];

    rp = real_roomp(v->in_room);
    if (rp && rp->dir_option[dir] &&
	rp->dir_option[dir]->to_room &&
	(EXIT(v, dir)->to_room != NOWHERE)) {
	if (v->specials.fighting) {
	    stop_fighting(v);
	}
	sprintf(buf, "%s picks you up and throws you %s\n\r",
		GET_NAME(ch), dirs[dir]);
	send_to_char(buf,v);
	or2 = v->in_room;
	char_from_room(v);
	char_to_room(v,(real_roomp(or2))->dir_option[dir]->to_room);
	do_look(v, "\0",15);

	if (IS_SET(RM_FLAGS(v->in_room), DEATH) && !IS_GOD(v))
	{
	    do_death_trap(v);
	}
	return TRUE;
    }
    return FALSE;
}


SPECIAL(ThrowerMob)
{
    struct char_data *vict;
    struct char_data *thrower = (struct char_data *) me;

    /*
     **  Throws people in various directions
     */

    if (type == SPEC_INIT)
	return (FALSE);

    if (!cmd) {
	if (AWAKE(thrower) && thrower->specials.fighting) {
	    /*
	     **  take this person and throw them
	     */
	    vict = thrower->specials.fighting;
	    switch(thrower->in_room) {
	    case 13912:
		return ThrowChar(thrower, vict, 1);	/* throw chars to the east */
		break;
	    default:
		return(FALSE);
	    }
	}
    } else {
	switch(thrower->in_room) {
	case 13912: {
	    if (cmd == 1) {	/* north */
		send_to_char("The Troll blocks your way.\n",thrower);
		return(TRUE);
	    }
	    break;
	}
	default:
	    return(FALSE);
	}
    }
    return(FALSE);
}


/* utilities used by lattimore */
/* Returns the index to the dude who did it */
int affect_status(struct memory *mem, struct char_data *ch,
		  struct char_data *t, int aff_status)
{
    int i;

    if(mem->c)
    {
	for(i = 0;i < mem->c; ++i)
	    if(!(strcmp(GET_NAME(t),mem->names[i])))
	    {
		mem->status[i] += aff_status;
		return(i);
		break;
	    }
	RECREATE(mem->names, char*, mem->c);
	RECREATE(mem->status, int, mem->c);
    }
    else
    {
	CREATE(mem->names, char*, 1);
	CREATE(mem->status, int, 1);
    }

    mem->names[mem->c] = strdup(GET_NAME(t));
    mem->status[mem->c] = aff_status;
    ++mem->c;
    return(mem->c-1);
}


char *lattimore_descs[] = {
    "A small orc is trying to break into a locker.\n\r",
    "A small orc is walking purposefully down the hall.\n\r",
    "An orc is feeding it's face with rat stew.\n\r",
    "A small orc is cowering underneath a bunk\n\r",
    "An orc sleeps restlessly on a bunk.\n\r",
    "There is an orc stading on a barrel here.\n\r",
    "An orc is traveling down the corridor at high speed.\n\r"
    };


SPECIAL(lattimore)
{
#define Lattimore_Initialize  0
#define Lattimore_Lockers     1
#define Lattimore_FoodRun     2
#define Lattimore_Eating      3
#define Lattimore_GoHome      4
#define Lattimore_Hiding      5
#define Lattimore_Sleeping    6
#define Lattimore_Run         7
#define Lattimore_Item        8

    struct memory *mem;
    struct char_data *t, *vict;
    struct obj_data *obj;
    char obj_name[MAX_INPUT_LENGTH], player_name[MAX_INPUT_LENGTH];
    struct char_data *latt = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (!cmd && !latt->hunt_info) {

	if(!latt->act_ptr) {
	    CREATE(mem, struct memory, 1);
	    mem->pointer = mem->c = mem->index = 0;
	    latt->act_ptr = (int) mem;
	}
	else mem = (struct memory *) latt->act_ptr;

	if (latt->master) {
	    mem->pointer = 0;
	    return(FALSE);
	}
	if(!AWAKE(latt)) return(FALSE);

	if (latt->specials.fighting) {
	    if(!IS_MOB(latt->specials.fighting) && CAN_SEE(latt,latt->specials.fighting))
		affect_status(mem, latt, latt->specials.fighting, -5);
	    if(mem->status[mem->index] < 0) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[6]);
		mem->pointer = Lattimore_Run;
	    }
	    else if(mem->status[mem->index] > 19) mem->pointer = Lattimore_Item;
	    return(FALSE);
	}

	switch(mem->pointer) {

	    /* This case is used at startup, and after player interaction*/
	case Lattimore_Initialize:

	    if((time_info.hours < 5) || (time_info.hours > 21)) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[3]);
		if(latt->in_room != Barracks) {
		    char_from_room(latt);
		    char_to_room(latt, Barracks);
		}
		mem->pointer = Lattimore_Hiding;
	    }
	    else if(time_info.hours < 11) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[4]);
		if(latt->in_room != Barracks) {
		    char_from_room(latt);
		    char_to_room(latt, Barracks);
		}
		mem->pointer = Lattimore_Sleeping;
	    }
	    else if((time_info.hours < 16) ||
		    ((time_info.hours > 17) && (time_info.hours < 22))) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[0]);
		if(latt->in_room != Barracks) {
		    char_from_room(latt);
		    char_to_room(latt, Barracks);
		}
		mem->pointer = Lattimore_Lockers;
	    }
	    else if(time_info.hours < 19) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[1]);
		mem->pointer = Lattimore_FoodRun;
	    }
	    break;

	case Lattimore_Lockers:

	    if(time_info.hours == 17) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[1]);
		mem->pointer = Lattimore_FoodRun;
	    }
	    else if(time_info.hours > 21) {
		act("$n cocks his head, as if listening.",
		    FALSE, latt, 0, 0, TO_ROOM);
		act("$n looks frightened, and dives under the nearest bunk.",
		    FALSE, latt, 0, 0, TO_ROOM);
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[3]);
		mem->pointer = Lattimore_Hiding;
		return TRUE;
	    }
	    break;

	case Lattimore_FoodRun:

	    if (latt->in_room != Kitchen) {
		track_to_room(latt, Kitchen, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		if(!latt->hunt_info)
		{
		    act("$n says 'Man, am I lost!'", FALSE, latt, 0, 0, TO_ROOM);
		    track_to_room(latt, Barracks, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		    if(!latt->hunt_info)
		    {
			char_from_room(latt);
			char_to_room(latt, Barracks);
		    }
		}
	    }
	    else {
		act("$n gets utensils off the counter, and ladels himself some stew."
		    , FALSE, latt, 0, 0, TO_ROOM);
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[2]);
		mem->pointer = Lattimore_Eating;
	    }
	    return TRUE;
	    break;

	case Lattimore_Eating:

	    if(time_info.hours > 18) {
		act("$n rubs his stomach and smiles happily.",
		    FALSE, latt, 0, 0, TO_ROOM);
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[1]);
		mem->pointer = Lattimore_GoHome;
	    }
	    else if(!number(0,2)) {
		act("$n gets some bread from the oven to go with his stew.",
		    FALSE, latt, 0, 0, TO_ROOM);
		act("$n dips the bread in the stew and eats it.",
		    FALSE, latt, 0, 0, TO_ROOM);
	    }
	    return TRUE;
	    break;

	case Lattimore_GoHome:

	    if (latt->in_room != Barracks) {
		track_to_room(latt, Barracks, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		if(!latt->hunt_info)
		{
		    act("$n says 'Man, am I lost!'", FALSE, latt, 0, 0, TO_ROOM);
		    if(!latt->hunt_info)
		    {
			char_from_room(latt);
			char_to_room(latt, Barracks);
		    }
		}
	    }
	    else {
		act("$n pulls out a crowbar and tries to open another locker.",
		    FALSE, latt, 0, 0, TO_ROOM);
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[0]);
		mem->pointer = Lattimore_Lockers;
	    }
	    return TRUE;
	    break;

	case Lattimore_Hiding:

	    if ((time_info.hours > 5) && (time_info.hours < 22)) {
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[4]);
		mem->pointer = Lattimore_Sleeping;
	    }
	    break;

	case Lattimore_Sleeping:

	    if (time_info.hours > 11) {
		act("$n awakens, rises and stretches with a yawn.",
		    FALSE, latt, 0, 0, TO_ROOM);
		act("$n pulls out a crowbar and tries to open another locker.",
		    FALSE, latt, 0, 0, TO_ROOM);
		ss_free(latt->player.long_descr);
		latt->player.long_descr = ss_make(lattimore_descs[0]);
		mem->pointer = Lattimore_Lockers;
	    }
	    return TRUE;
	    break;

	case Lattimore_Run:

	    if (latt->in_room != Storeroom && latt->in_room != Trap) {
		if(latt->in_room == EarthQ) return(FALSE);
		track_to_room(latt, Storeroom, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		if(!latt->hunt_info)
		{
		    act("$n says 'Man, am I lost!'", FALSE, latt, 0, 0, TO_ROOM);
		    track_to_room(latt, Kitchen, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		    if(!latt->hunt_info)
		    {
			char_from_room(latt);
			char_to_room(latt, Barracks);
		    }
		}
	    }
	    else if(latt->in_room == Trap) {
		if(!IS_AFFECTED(latt,AFF_FLYING)) {
		    /* Get him up off the floor */
		    act("$n grins evilly, and quickly stands on a barrel.",
			FALSE, latt, 0, 0, TO_ROOM);
		    SET_BIT(AFF_FLAGS(latt), AFF_FLYING);
		    ss_free(latt->player.long_descr);
		    latt->player.long_descr = ss_make(lattimore_descs[5]);
		    mem->index = 0;
		}
		else ++mem->index;
		/* Wait a while, then go home */
		if(mem->index == 50) {
		    go_direction(latt, 1, 0);
		    mem->pointer = Lattimore_GoHome;
		    REMOVE_BIT(AFF_FLAGS(latt), AFF_FLYING);
		    ss_free(latt->player.long_descr);
		    latt->player.long_descr = ss_make(lattimore_descs[1]);
		}
	    }
	    return TRUE;
	    break;

	case Lattimore_Item:

	    if (latt->in_room != Conf) {
		track_to_room(latt, Conf, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		if(!latt->hunt_info)
		{
		    act("$n says 'Man, am I lost!'", FALSE, latt, 0, 0, TO_ROOM);
		    track_to_room(latt, Barracks, 100, HUNT_GLOBAL|HUNT_THRU_DOORS);
		    if(!latt->hunt_info)
		    {
			char_from_room(latt);
			char_to_room(latt, Barracks);
		    }
		}
	    }
	    else {
		for(t=real_roomp(latt->in_room)->people;t;t=t->next_in_room)
		    if(!IS_NPC(t) && CAN_SEE(latt,t))
			if(!(strcmp(mem->names[mem->index],GET_NAME(t)))) {
			    act("$n crawls under the large table.",
				FALSE, latt, 0, 0, TO_ROOM);
			    if(!(obj = make_object(PostKey, VIRTUAL)))
				return FALSE;
			    if ((IS_CARRYING_N(t)+1) < CAN_CARRY_N(t)) {
				act("$N emerges with $p, and gives it to you.",
				    FALSE, t, obj, latt, TO_CHAR);
				act("$n emerges with $p, and gives it to $N.",
				    FALSE, latt, obj, t, TO_ROOM);
				obj_to_char(obj,t);
			    }
			    else {
				act("$n emerges with $p, and drops it for $N.",
				    FALSE, latt, obj, t, TO_ROOM);
				obj_to_room(obj,latt->in_room);
			    }
			}
		/* Dude's not here - oh well, go home. */
		mem->status[mem->index] = 0; /* Duty discharged */
		mem->pointer = Lattimore_GoHome;
	    }
	    return TRUE;
	    break;

	default:
	    mem->pointer = Lattimore_Initialize;
	    break;

	}
    }
    else if(cmd == 72) {
	arg = one_argument(arg,obj_name);
	if ((!*obj_name)||!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
	    return(FALSE);
	only_argument(arg, player_name);
	if((!*player_name) || (!(vict = get_char_room_vis(ch, player_name))))
	    return(FALSE);
	/* the target is Lattimore */
	if (vict == latt)
	{
	    if(!latt->act_ptr) {
		CREATE(mem, struct memory, 1);
		ch->act_ptr = (int) mem;
		mem->pointer = mem->c = mem->index = 0;
	    }
	    else mem = (struct memory *) latt->act_ptr;

	    act("You give $p to $N.",TRUE, ch, obj, latt, TO_CHAR);
	    act("$n gives $p to $N.",TRUE, ch, obj, latt, TO_ROOM);

	    switch(obj->obj_flags.type_flag) {

	    case ITEM_FOOD:
		if(obj->obj_flags.value[3]) {
		    act("$n sniffs $p, then discards it with disgust.",
			TRUE, latt, obj, 0, TO_ROOM);
		    obj_from_char(obj);
		    obj_to_room(obj,ch->in_room);
		    if(!IS_MOB(ch) && CAN_SEE(latt,ch))
			mem->index = affect_status(mem, latt, ch, -5);
		    else return(TRUE);
		}
		else {
		    act("$n takes $p and hungrily wolfs it down.",
			TRUE, latt, obj, 0, TO_ROOM);
		    extract_obj(obj);
		    if(!IS_MOB(ch) && CAN_SEE(latt,ch))
			mem->index = affect_status(mem, latt, ch, 4);
		    else return(TRUE);
		}
		break;
	    case ITEM_KEY:
		/* What he really wants */
		if(obj_index[obj->item_number].virt == CrowBar) {
		    act("$n takes $p and jumps up and down in joy.",
			TRUE, latt, obj, 0, TO_ROOM);
		    obj_from_char(obj);
		    if (!ch->equipment[HOLD]) equip_char(ch, obj, HOLD);
		    if(!IS_MOB(ch) && CAN_SEE(latt,ch))
			mem->index = affect_status(mem, latt, ch, 20);
		    else return(TRUE);
		}
		break;
	    default:
		/* Any other types of items */
		act("$n looks at $p curiously.", TRUE, latt, obj, 0, TO_ROOM);
		if(!IS_MOB(ch) && CAN_SEE(latt,ch))
		    mem->index = affect_status(mem, latt, ch, 1);
		else return(TRUE);
		break;
	    }
	    /* They gave something to him, and the status was affected,
	       now we set the pointer according to the status value */
	    if(mem->status[mem->index] < 0) {
		ss_free(ch->player.long_descr);
		ch->player.long_descr = ss_make(lattimore_descs[6]);
		mem->pointer = Lattimore_Run;
	    }
	    else if(mem->status[mem->index] > 19) {
		ss_free(ch->player.long_descr);
		ch->player.long_descr = ss_make(lattimore_descs[6]);
		mem->pointer = Lattimore_Item;
	    }
	    return(TRUE);
	}
	return(FALSE);
    }
    return(FALSE);
}


SPECIAL(trapper)
{
    struct char_data *tch;
    struct char_data *trapper = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(trapper)) return (FALSE);

    /* Okay, the idea is this: If the PC or NPC in this room isn't flying,
       it is walking on the trapper. Doesn't matter if it's sneaking, or
       invisible, or whatever. The trapper will attack both PCs and NPCs,
       so don't have a lot of wandering NPCs around it. */

    if (!trapper->specials.fighting) {
	for (tch=real_roomp(trapper->in_room)->people;tch;tch=tch->next_in_room)
	    if((trapper != tch) && !IS_IMMORTAL(tch) && !IS_AFFECTED(tch,AFF_FLYING)) {
		set_fighting(trapper,tch);
		return(TRUE);
	    }
	/* Nobody here */
	return(FALSE);
    }
    else {
	if (IS_IMMORTAL(trapper->specials.fighting))
	    return(FALSE);

	/* Equipment must save against crush - will fail 25% of the time */
	DamageStuff(trapper->specials.fighting,TYPE_CRUSH,number(0,7));

	/* Make the poor sucker save against paralzyation, or suffocate */
	if(saves_spell(trapper->specials.fighting,SAVING_PARA, 0)) {
	    act("You can hardly breathe, $N is suffocating you!",
		FALSE, trapper->specials.fighting, 0, trapper, TO_CHAR);
	    act("$N is stifling $n, who will suffocate soon!",
		FALSE, trapper->specials.fighting, 0, trapper, TO_ROOM);
	}
	else {
	    act("You gasp for air inside $N!",
		FALSE, trapper->specials.fighting, 0, trapper, TO_CHAR);
	    act("$N stifles you. You asphyxiate and die!",
		FALSE, trapper->specials.fighting, 0, trapper, TO_CHAR);
	    act("$n has suffocated inside $N!",
		FALSE, trapper->specials.fighting, 0, trapper, TO_ROOM);
	    act("$n is dead!", FALSE, trapper->specials.fighting, 0, trapper, TO_ROOM);
	    die(trapper->specials.fighting);
	}
	return(TRUE);
    }
}


SPECIAL(trogcook)
{
    struct char_data *tch;
    struct char_data *cook = (struct char_data *) me;
    struct obj_data *corpse;
    char buf[MAX_INPUT_LENGTH];

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(cook)) return (FALSE);

    if (cook->specials.fighting) {
	if (GET_POS(cook) != POSITION_FIGHTING) StandUp(cook);
	return (FALSE);
    }

    for (tch=real_roomp(cook->in_room)->people; tch; tch = tch->next_in_room)
	if(IS_NPC(tch) && IsAnimal(tch) && CAN_SEE(cook, tch)) {
	    if (!check_soundproof(cook))
		act("$n cackles 'Something else for the pot!'",FALSE,cook,0,0,TO_ROOM);
	    hit(cook, tch, TYPE_UNDEFINED);
	    return (TRUE);
	}

    corpse = get_obj_in_list_vis(cook,"corpse",real_roomp(cook->in_room)->contents);
    if (corpse)
    {
	do_get(cook, "corpse", -1	/* irrelevant */);
	act("$n cackles 'Into the soup with it!'", FALSE, cook, 0, 0, TO_ROOM);
	sprintf(buf, "put corpse pot");
	command_interpreter(cook, buf, 1);
	return(TRUE);
    }

    return FALSE;
}


SPECIAL(shaman)
{
    struct char_data *god, *tch;
    struct char_data *shaman = (struct char_data *) me;

    switch (type)
    {
    case SPEC_CMD:
	return (FALSE);

    case SPEC_FIGHT:
	if(number(0,3) == 0)
	{
	    for (tch = real_roomp(shaman->in_room)->people; tch;
		 tch = tch->next_in_room)
		if ((!IS_NPC(tch)) && (GetMaxLevel(tch) > 20) &&
		    CAN_SEE(shaman, tch))
		{
		    if(!(god = get_char_room_vis(shaman, DEITY_NAME)))
		    {
			act("$n screams 'Golgar, I summon thee to aid thy servants!'",
			    FALSE, shaman, 0, 0, TO_ROOM);
			if(number(0,8) == 0)
			{
			    act("There is a blinding flash of light!",
				FALSE, shaman, 0, 0, TO_ROOM);
			    god = make_mobile(VMOB_DEITY, VIRTUAL);
			    char_to_room(god, shaman->in_room);
			}
		    }
		    else if(number(0,2) == 0)
			act("$n shouts 'Now you will die!'",
			    FALSE, shaman, 0, 0, TO_ROOM);
		    return TRUE;
		}
	}
	/* fall through */

    default:
	return(generic_cleric(me, ch, cmd, arg, type));
    }

    return FALSE;
}


SPECIAL(golgar)
{
    struct char_data *shaman, *tch;
    struct char_data *golgar = (struct char_data *) me;

    switch (type)
    {
    case SPEC_IDLE:
	if(!(shaman = get_char_room_vis(golgar, SHAMAN_NAME)))
	{
	    for (tch = real_roomp(golgar->in_room)->people; tch;
		 tch = tch->next_in_room)
		if (IS_NPC(tch) && (GET_RACE(tch) == RACE_TROGLODYTE))
		    if((tch->specials.fighting) &&
		       (!IS_NPC(tch->specials.fighting)))
		    {
			act("$n growls 'Death to those attacking my people!'",
			    FALSE, golgar, 0, 0, TO_ROOM);
			hit(golgar, tch->specials.fighting, TYPE_UNDEFINED);
			return TRUE;
		    }
	    if(number(0,5) == 0)
	    {
		act("$n slowly fades into ethereal emptiness.",
		    FALSE, golgar, 0, 0, TO_ROOM);
		extract_char(golgar);
	    }
	}
	else
	{
	    if(!shaman->specials.fighting)
	    {
		act("$n growls 'How dare you summon me!'",
		    FALSE, golgar, 0, 0, TO_ROOM);
		hit(golgar, shaman, TYPE_UNDEFINED);
	    }
	    else
	    {
		act("$n screams 'You dare touch my holy messenger!? DIE!'",
		    FALSE, golgar, 0, 0, TO_ROOM);
		hit(golgar, shaman->specials.fighting, TYPE_UNDEFINED);
	    }
	    return TRUE;
	}

    default:
	return(magic_user(me, ch, cmd, arg, type));
    }

    return (FALSE);
}


SPECIAL(troguard)
{
    struct char_data *tch, *good;
    struct char_data *guard = (struct char_data *) me;
    int max_good;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(guard))
	return (FALSE);

  if (guard->specials.fighting) {
    if (GET_POS(guard) == POSITION_FIGHTING) {
      FighterMove(guard);
    } else {
      StandUp(guard);
    }

    if (!check_soundproof(guard)) {
      act("$n shouts 'The enemy is upon us! Help me, my brothers!'",
	  TRUE, guard, 0, 0, TO_ROOM);
      if (guard->specials.fighting)
	CallForGuard(guard, guard->specials.fighting, 3, TROGCAVES);
    }
    return(TRUE);
  }

  max_good = -1001;
  good = 0;

  for (tch=real_roomp(guard->in_room)->people; tch; tch = tch->next_in_room)
    if ((GET_ALIGNMENT(tch) > max_good) && !IS_IMMORTAL(tch) &&
	(!IS_NPC(tch) || (IS_NPC(tch) && (GET_RACE(tch) != RACE_TROGLODYTE))))
    {
      max_good = GET_ALIGNMENT(tch);
      good = tch;
    }

  if (check_peaceful(guard, ""))
    return FALSE;

  if (good) {
    if (!check_soundproof(guard))
      act("$n screams 'Die invading scum! Take that!'",
	  FALSE, guard, 0, 0, TO_ROOM);
    hit(guard, good, TYPE_UNDEFINED);
    return(TRUE);
  }

  return(FALSE);
}


SPECIAL(keystone)
{
    /* Must be a unique identifier for this mob type, or we lose */
#define Identifier      "gds"

    struct char_data *ghost, *t, *master;
    struct char_data *stone = (struct char_data *) me;
    int i;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(stone)) return(FALSE);

    if(time_info.hours == 22) {
	if(!(ghost = get_char_vis_world(stone, Identifier, 0))) {
	    act("$n cries 'Awaken my soldiers! Our time is nigh!'",
		FALSE, stone, 0, 0, TO_ROOM);
	    act("You suddenly feel very, very afraid.",
		FALSE, stone, 0, 0, TO_ROOM);
	    for (i = START_ROOM;i < END_ROOM; ++i)
		if(number(0,2) == 0) {
		    ghost = make_mobile(GhostSoldier, VIRTUAL);
		    char_to_room(ghost, i);
		}
		else if(number(0,7) == 0) {
		    ghost = make_mobile(GhostLieutenant, VIRTUAL);
		    char_to_room(ghost, i);
		}
	    EACH_CHARACTER(iter, t)
	    {
		if (real_roomp(stone->in_room)->zone == real_roomp(t->in_room)->zone)
		    act("You hear a strange cry that fills your soul with fear!",
			FALSE, t, 0, 0, TO_CHAR);
	    }
	    END_AITER(iter);
	    return TRUE;
	}
    }

    if(stone->specials.fighting) {
	if(IS_NPC(stone->specials.fighting) &&
	   !IS_SET((stone->specials.fighting)->specials.mob_act,ACT_POLYSELF))
	    if((master = (stone->specials.fighting)->master) && CAN_SEE(stone,master)) {
		stop_fighting(stone);
		hit(stone, master, TYPE_UNDEFINED);
	    }
	if (GET_POS(stone) == POSITION_FIGHTING) {
	    FighterMove(stone);
	} else {
	    StandUp(stone);
	}
	CallForGuard(stone, stone->specials.fighting, 3, OUTPOST);
	return TRUE;
    }

    return(FALSE);
}


SPECIAL(ghostsoldier)
{
    struct char_data *solider = (struct char_data *) me;
    struct char_data *tch, *good, *master;
    int max_good;
    spec_proc_func gs, gc;

    gs = ghostsoldier;
    gc = keystone;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd) return(FALSE);

    if(time_info.hours > 4 && time_info.hours < 22) {
	act("$n slowly fades out of existence.", FALSE, solider, 0, 0, TO_ROOM);
	extract_char(solider);
	return(TRUE);
    }

    max_good = -1001;
    good = 0;

    for (tch=real_roomp(solider->in_room)->people; tch; tch = tch->next_in_room)
	if (!(mob_index[tch->nr].func == gs) && /* Another ghost soldier? */
	    !(mob_index[tch->nr].func == gc) && /* The ghost captain? */
	    (GET_ALIGNMENT(tch) > max_good) && /* More good than prev? */
	    !IS_IMMORTAL(tch) && /* A god? */
	    (GET_RACE(tch) >= 4)) { /* Attack only npc races */
	    max_good = GET_ALIGNMENT(tch);
	    good = tch;
	}

    /* What is a ghost Soldier doing in a peaceful room? */
    if (check_peaceful(solider, ""))
	return FALSE;

    if (good) {
	if (!check_soundproof(solider))
	    act("$N attacks you with an unholy scream!",FALSE, good, 0, solider, TO_CHAR);
	hit(solider, good, TYPE_UNDEFINED);
	return(TRUE);
    }

    if(solider->specials.fighting) {
	if(IS_NPC(solider->specials.fighting) &&
	   !IS_SET((solider->specials.fighting)->specials.mob_act,ACT_POLYSELF))
	    if((master = (solider->specials.fighting)->master) && CAN_SEE(solider,master)) {
		stop_fighting(solider);
		hit(solider, master, TYPE_UNDEFINED);
	    }
	if (GET_POS(solider) == POSITION_FIGHTING) {
	    FighterMove(solider);
	} else {
	    StandUp(solider);
	}
	CallForGuard(solider, solider->specials.fighting, 3, OUTPOST);
	return TRUE;
    }
    return(FALSE);
}


SPECIAL(guardian)
{
#define RHYODIN_FILE "rhyodin"

    FILE *pass;
    struct char_data *g, *master;
    struct obj_data *obj;
    struct room_data *rp;
    struct follow_type *fol;
    char player_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct char_data *guardian = (struct char_data *) me;

    struct Names {
	char **names;
	short num_names;
    } *gstruct;

    int j = 0;

    if (type == SPEC_INIT)
	return (FALSE);

    if((cmd && !((cmd == 72) || (cmd == 3))) || !AWAKE(guardian))
	return(FALSE);

    if(!cmd && (guardian->act_ptr == -1)) return(FALSE);

    else if(!cmd && !guardian->act_ptr) {

	/* Open the file, read the names into an array in the act pointer */
	if(!(pass = fopen(RHYODIN_FILE, "r"))) {
	    log_msg("Rhyodin access file unreadable or non-existant", LOG_MPROG);
	    guardian->act_ptr = -1;
	    return(FALSE);
	}

	CREATE(gstruct, struct Names, 1);
	guardian->act_ptr = (long) gstruct;
	CREATE(gstruct->names, char*, 1);
	gstruct->num_names = 0;

	while (1 == fscanf(pass, " %s\n", name)) {
	    ++gstruct->num_names;
	    RECREATE(gstruct->names, char*, gstruct->num_names);
	    gstruct->names[gstruct->num_names - 1] = strdup(name);
	}
    }
    else gstruct = (struct Names *) guardian->act_ptr;

    if(!cmd) {
	if(guardian->specials.fighting) {
	    if(IS_NPC(guardian->specials.fighting) &&
	       !IS_SET((guardian->specials.fighting)->specials.mob_act,ACT_POLYSELF))
		if((master = (guardian->specials.fighting)->master) && CAN_SEE(guardian,master)) {
		    stop_fighting(guardian);
		    hit(guardian, master, TYPE_UNDEFINED);
		}
	    if (GET_POS(guardian) == POSITION_FIGHTING) {
		FighterMove(guardian);
	    } else {
		StandUp(guardian);
	    }
	}
	return TRUE;
    }

    if(cmd == 72) {
	arg = one_argument(arg,obj_name);
	if ((!*obj_name)||!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
	    return(FALSE);
	only_argument(arg, player_name);
	if((!*player_name) || (!(g = get_char_room_vis(ch, player_name))))
	    return(FALSE);

	if (g == guardian)
	{
	    gstruct = (struct Names *) g->act_ptr;
	    act("You give $p to $N.",TRUE, ch, obj, g, TO_CHAR);
	    act("$n gives $p to $N.",TRUE, ch, obj, g, TO_ROOM);

	    if (obj_index[obj->item_number].virt == Necklace) {

		if(!check_soundproof(ch)) {
		    act("$n takes $p, and unlocks the gate.",
			FALSE, g, obj, 0, TO_ROOM);
		    act("$p pulses in his hand, and disappears.",
			FALSE, g, obj, 0, TO_ROOM);
		    act("$N says 'You have proven youself worthy.'",
			FALSE, ch, 0, g, TO_CHAR);
		    act("$N says 'You are now an ambassador from the north to Rhyodin.'",
			FALSE, ch, 0, g, TO_CHAR);
		}

		/* Take it away */
		obj_from_char(obj);
		extract_obj(obj);

		if(!IS_NPC(ch)) {
		    if(!(pass = fopen(RHYODIN_FILE, "a"))) {
			log_msg("Couldn't open file for writing permanent Rhyodin passlist.", LOG_MPROG);
			return(FALSE);
		    }
		    /* Go to the end of the file and write the characters name */
		    fprintf(pass, " %s\n", GET_NAME(ch));
		    fclose(pass);
		}

		/* Okay, now take person and all followers in this room to next room */
		act("$N opens the gate and guides you through.",
		    FALSE, ch, 0, g, TO_CHAR);
		rp = real_roomp(ch->in_room);

		char_from_room(ch);
		char_to_room(ch,rp->dir_option[2]->to_room);
		do_look(ch, "\0", 0);

		/* First level followers can tag along */
		if(ch->followers) {
		    act("$N says 'If they're with you, they can enter as well.'",
			FALSE, ch, 0, g, TO_CHAR);
		    for(fol = ch->followers ; fol ;fol = fol->next) {
			if (fol->follower->specials.fighting) continue;
			if (real_roomp(fol->follower->in_room) &&
			    (EXIT(fol->follower,2)->to_room != NOWHERE) &&
			    (GET_POS(fol->follower) >= POSITION_STANDING)) {
			    char_from_room(fol->follower);
			    char_to_room(fol->follower,rp->dir_option[2]->to_room);
			    do_look(fol->follower, "\0", 0);
			}
		    }
		}
		return(TRUE);
	    }
	}
    }
    else if (cmd == 3 && !IS_NPC(ch)) {
	g = guardian;
	gstruct = (struct Names *) g->act_ptr;
	j = 0;

	/* Trying to move south, check against namelist */
	while(j < gstruct->num_names) {
	    if(!(strcmp(gstruct->names[j],GET_NAME(ch))))
		if (real_roomp(ch->in_room) && (EXIT(ch, 2)->to_room != NOWHERE)) {
		    if (ch->specials.fighting) return(FALSE);
		    act("$N recognizes you, and escorts you through the gate.",
			FALSE, ch, 0, g, TO_CHAR);
		    act("$N recognizes $n, and escorts them through the gate.",
			FALSE, ch, 0, g, TO_ROOM);
		    rp = real_roomp(ch->in_room);
		    char_from_room(ch);
		    char_to_room(ch,rp->dir_option[2]->to_room);
		    do_look(ch, "\0", 0);
		    /* Follower stuff again */
		    if(ch->followers) {
			act("$N says 'If they're with you, they can enter as well.'",
			    FALSE, ch, 0, g, TO_CHAR);
			for(fol = ch->followers ; fol ;fol = fol->next) {
			    if (fol->follower->specials.fighting) continue;
			    if (real_roomp(fol->follower->in_room) &&
				(EXIT(fol->follower,2)->to_room != NOWHERE) &&
				(GET_POS(fol->follower) >= POSITION_STANDING)) {
				char_from_room(fol->follower);
				char_to_room(fol->follower,rp->dir_option[2]->to_room);
				do_look(fol->follower, "\0", 0);
			    }
			}
		    }
		    return(TRUE);
		}
	    ++j;
	}
    }
    return(FALSE);
}

/****************************************/
/*** new and updated mob procs by bjh ***/
/****************************************/

SPECIAL(lone_troll)
{
    struct obj_data *i, *temp, *next_obj, *next_r_obj;
    struct char_data *troll = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(troll) || troll->specials.fighting)
	return(FALSE);

    /*** go through contents of room ***/
    for (i=real_roomp(troll->in_room)->contents; i; i=next_r_obj) {
	next_r_obj=i->next_content;

	/*** check for a corpse ***/
	if (IS_CORPSE(i)) {
	    act("$n plunders a corpse and then hungrily devours it!",
		FALSE, troll, 0, 0, TO_ROOM);
	    if(IS_PC_CORPSE(i)) remove("corpsedata.dat");

	    /*** go through items contained in the corpse ***/
	    for (temp=i->contains; temp; temp=next_obj) {
		next_obj=temp->next_content;

		/*** take item from corpse if possible ***/
		if (IS_SET(temp->obj_flags.wear_flags, ITEM_TAKE)) {
		    obj_from_obj(temp);
		    obj_to_char(temp,troll);

		    /*** wear item if not a weapon ***/
		    if((!CAN_WEAR(temp, ITEM_WIELD)) &&
		       (can_wear_test(troll,temp)))
			do_wear(troll,fname(OBJ_NAME(temp)),0);

		    /*** cannot take item, so transfer from corpse to room ***/
		} else {
		    obj_from_obj(temp);
		    obj_to_room(temp,troll->in_room);
		}
	    }
	    /*** destroy the corpse ***/
	    extract_obj(i);
	    return(TRUE);
	}
    }
    return(FALSE);
}


SPECIAL(tyrsis_magician)
{
    struct char_data *mage = (struct char_data *) me;

    if (type == SPEC_FIGHT && number(0, 1))
    {
	command_interpreter(mage, "use ball", 1);
	return (TRUE);
    }

    return (magic_user2(me, ch, cmd, arg, type));
}


SPECIAL(snake)
{
  struct char_data *vict;
  struct char_data *snake = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(snake))
    return(FALSE);

  if(GET_POS(snake)<POSITION_FIGHTING) {
    if (IS_PC(snake))
      return (FALSE);
    StandUp(snake);
    return (TRUE);
  }

  if (!(vict=snake->specials.fighting))
    return (FALSE);

  act("$n strikes at $N!", TRUE, snake, 0, vict, TO_NOTVICT);
  act("$n strikes at you!", TRUE, snake, 0, vict, TO_VICT);
  cast_poison( GetMaxLevel(snake), snake, "", SPELL_TYPE_SPELL, vict, 0);
  return (TRUE);
}


SPECIAL(blink)
{
    struct char_data *dog = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(dog))
	return(FALSE);

    if(GET_POS(dog)<POSITION_FIGHTING) {
	if (IS_PC(dog))
	    return (FALSE);
	StandUp(dog);
	return (TRUE);
    }

    if (GET_HIT(dog) < GET_MAX_HIT(dog)/3)
    {
	act("$n blinks!",TRUE,dog,0,0,TO_ROOM);
	cast_teleport(GetMaxLevel(dog), dog, "", SPELL_TYPE_SPELL, dog, 0);
	return(TRUE);
    }

    return(FALSE);
}


SPECIAL(ghoul)
{
    struct char_data *vict;
    struct char_data *ghoul = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(ghoul))
	return(FALSE);

    if (!(vict=ghoul->specials.fighting))
	return (FALSE);

    if(GET_POS(ghoul)<POSITION_FIGHTING) {
	if (IS_PC(ghoul))
	    return (FALSE);
	StandUp(ghoul);
	return (TRUE);
    }

    act("$n touches $N!", TRUE, ghoul, 0, vict, TO_NOTVICT);
    act("$n touches you!", TRUE, ghoul, 0, vict, TO_VICT);
    cast_paralyze(GetMaxLevel(ghoul), ghoul, "", SPELL_TYPE_SPELL, vict, 0);
    return (TRUE);
}


SPECIAL(CarrionCrawler)
{
  struct char_data *vict;
  struct char_data *crawler = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(crawler))
    return(FALSE);

  if (!(vict=FindVictim(crawler)))
    return (FALSE);

  if(GET_POS(crawler)<POSITION_FIGHTING) {
    if (IS_PC(crawler))
      return (FALSE);
    StandUp(crawler);
    return (TRUE);
  }

  act("$n touches $N!", TRUE, crawler, 0, vict, TO_NOTVICT);
  act("$n touches you!", TRUE, crawler, 0, vict, TO_VICT);
  cast_paralyze(GetMaxLevel(crawler), crawler, "", SPELL_TYPE_SPELL, vict, 0);
  return (TRUE);
}


SPECIAL(vampire)
{
  struct char_data *vict;
  struct char_data *vampire = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(vampire))
    return(FALSE);

  if (!(vict=vampire->specials.fighting))
    return (FALSE);

  if(GET_POS(vampire)<POSITION_FIGHTING) {
    if (IS_PC(vampire))
      return (FALSE);
    StandUp(vampire);
    return (TRUE);
  }
/*
  act("$n grasps $N!", TRUE, vampire, 0, vict, TO_NOTVICT);
  act("$n grasps you!", TRUE, vampire, 0, vict, TO_VICT); */
  cast_vampyric_touch(GetMaxLevel(vampire), vampire, "", SPELL_TYPE_SPELL, vict, 0);

  return (TRUE);
}


SPECIAL(wraith)
{
  struct char_data *vict;
  struct char_data *wraith = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(wraith))
    return(FALSE);

  if (!(vict=wraith->specials.fighting))
    return (FALSE);

  if(GET_POS(wraith)<POSITION_FIGHTING) {
    if (IS_PC(wraith))
      return (FALSE);
    StandUp(wraith);
    return (TRUE);
  }

  act("$n touches $N!", TRUE, wraith, 0, vict, TO_NOTVICT);
  act("$n touches you!", TRUE, wraith, 0, vict, TO_VICT);
  cast_energy_drain( GetMaxLevel(wraith), wraith, "", SPELL_TYPE_SPELL, vict, 0);

  return (TRUE);
}


SPECIAL(shadow)
{
  struct char_data *vict;
  struct char_data *shadow = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(shadow))
    return(FALSE);

  if (!(vict=shadow->specials.fighting))
    return (FALSE);

  if(GET_POS(shadow)<POSITION_FIGHTING) {
    if (IS_PC(shadow))
      return (FALSE);
    StandUp(shadow);
    return (TRUE);
  }

  act("$n touches $N!", TRUE, shadow, 0, vict, TO_NOTVICT);
  act("$n touches you!", TRUE, shadow, 0, vict, TO_VICT);
  cast_chill_touch(GetMaxLevel(shadow), shadow, "", SPELL_TYPE_SPELL, vict, 0);

  return (TRUE);
}

/*************************************************************************/
/*** the following are semi-smart procs for various "type" casters     ***/
/*** the best spell is choosen from those available, instead of random ***/
/*************************************************************************/

int bestlevel(int spellno);
int choose_best(struct char_data *ch, int spell_list[])
{
  int x, temp, best;

  best=spell_list[0];
  x=1;
  while (spell_list[x]) {
    temp=bestlevel(spell_list[x]);
    if (GetMaxLevel(ch)>=temp && temp>bestlevel(best))
      best=spell_list[x];
    x++;
  }

  return (best);
}

int typecaster(struct char_data *ch, int area_affect, int spell_list[],
               char *buf, int cmd, int type)
{
  struct char_data *vict;
  struct spell_info *ability;
  int choice;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (!(vict=ch->specials.fighting))
    return (FALSE);

  if(GET_POS(ch)<POSITION_FIGHTING) {
    if (IS_PC(ch))
      return (FALSE);
    if(GET_POS(ch) > POSITION_SITTING)
	return FALSE;
    StandUp(ch);
    return (TRUE);
  }

  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  choice=0;
  if (area_affect>0)
    if (ch->attackers>1 && GetMaxLevel(ch)>=bestlevel(area_affect))
      choice=area_affect;

  if (choice==0)
    choice=choose_best(ch, spell_list);

  ability=spell_by_number(choice);
  (ability->spell_pointer)(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

  return(TRUE);
}

/*  Added by Novak, so remove if wrong :)  */

SPECIAL(acidcaster)
{
  int spell_list[]={SPELL_ACID_BLAST, 0};

  return (typecaster((struct char_data *) me, SPELL_ACID_RAIN,
		     spell_list, "$n calls upon $s acidic abilities!", cmd, type));
}

SPECIAL(energycaster)
{
	int spell_list[]={SPELL_ENERGY_DRAIN, SPELL_RUPTURE,
		SPELL_IMPLODE, SPELL_WITHER,
		SPELL_VAMPYRIC_TOUCH, SPELL_DISINTEGRATE, 0};

	return (typecaster((struct char_data *) me, 0,
		spell_list, "$n calls upon $s enegetic abilities!", cmd,
type));
}

SPECIAL(poisoncaster)
{
	int spell_list[]={SPELL_POISON, SPELL_PARALYSIS, 0};

	return (typecaster((struct char_data *) me, SPELL_POISON_GAS,
		spell_list, "$n calls upon $s toxic abilities!",
	        cmd, type));
}

/*  end of Novaks stuff here */

SPECIAL(coldcaster)
{
  int spell_list[]={SPELL_CHILL_TOUCH, SPELL_FROST_CLOUD, 0};

  return (typecaster((struct char_data *) me, SPELL_ICE_STORM, spell_list,
		     "$n calls upon $s cold abilities!", cmd, type));
}


SPECIAL(firecaster)
{
  int spell_list[]={SPELL_BURNING_HANDS, SPELL_FIRE_WIND, SPELL_FLAMESTRIKE,
                    SPELL_FIREBALL, 0};

  return (typecaster((struct char_data *) me, SPELL_LAVA_STORM, spell_list,
		     "$n draws upon $s fiery powers!", cmd, type));
}


SPECIAL(eleccaster)
{
  int spell_list[]={SPELL_SHOCKING_GRASP, SPELL_ELECTROCUTE,
                    SPELL_ELECTRIC_FIRE, 0};

  return (typecaster((struct char_data *) me, SPELL_CHAIN_ELECTROCUTION,
		     spell_list, "$n calls upon $s electric abilities!", cmd,
		     type));
}


SPECIAL(web_slinger)
{
  struct char_data *vict;
  struct char_data *slinger = (struct char_data *) me;

  if (type == SPEC_INIT)
      return (FALSE);

  if (cmd || !AWAKE(slinger))
    return(FALSE);

  if (!(vict=slinger->specials.fighting))
    return (FALSE);

  if(GET_POS(slinger)<POSITION_FIGHTING) {
    if (IS_PC(slinger))
      return (FALSE);
    StandUp(slinger);
    return (TRUE);
  }

  act("$n throws sticky webs at $N!", TRUE, slinger, 0, vict, TO_NOTVICT);
  act("$n throws sticky webs at you!", TRUE, slinger, 0, 0, TO_VICT);
  cast_web(GetMaxLevel(slinger), slinger, "", SPELL_TYPE_SPELL, vict, 0);

  return (TRUE);
}

/*** uses psi mindblast skill ***/

SPECIAL(mindflayer)
{
  int real_mana, flag=FALSE;
  struct char_data *flayer = (struct char_data *) me;

  if (type == SPEC_INIT)
      return FALSE;

  if (cmd || !AWAKE(flayer))
    return (FALSE);

  if (!flayer->specials.fighting || GET_POS(flayer)<POSITION_FIGHTING)
    return (FALSE);

  if (!flayer->skills)
    SpaceForSkills(flayer);

  if (!(flayer->player.clss & CLASS_PSI)) { /* should'nt be necessary, but */
    flayer->player.clss += CLASS_PSI;       /* will do it just to be safe  */
    flayer->player.level[PSI_LEVEL_IND]=GetMaxLevel(flayer);
    flayer->skills[SKILL_PSIONIC_BLAST].learned=101;
    flag=TRUE;
  }

  real_mana=flayer->points.mana;
  flayer->points.mana=25;

  do_psi_attack (flayer, "", 389);

  flayer->points.mana=real_mana;

  if (flag) {
    flayer->player.clss -= CLASS_PSI;
    flayer->player.level[PSI_LEVEL_IND]=0;
    flayer->skills[SKILL_PSIONIC_BLAST].learned=0;
  }

  return TRUE;
}

/*************************/
/*** Guildmaster procs ***/
/*************************/

int GuildMaster(struct char_data* ch, int cmd, char* arg,
		int clss, struct char_data* mast)
{
    int	mask = 1 << clss;
    int max = GET_LEVEL(mast, clss);

    if(cmd != 243 && cmd != 170 && cmd != 164)
	return FALSE;

    if(!mast ||
       check_soundproof(ch) ||
       IS_IMMORTAL(ch) ||
       !ch->skills ||
       !HasClass(ch, mask))
	return FALSE;

    if(!HasClass(mast, mask))
      max = GetMaxLevel(mast);

    /*  Temporary shit */
    if(max == 50)
	max = 200;

    switch(cmd)
    {
    case 243:			/* gain */
      if(GET_LEVEL(ch, clss) <= max)
      {
	GainLevel(ch, clss);
	UpdateMaxLevel(ch);
	UpdateMinLevel(ch);

      }
      else
	send_to_char("I cannot train you.\n\r", ch);
      break;

    case 170:
    case 164:			/* practice */
      while(*arg && *arg == ' ')
	arg++;
      if(!*arg)
	ShowPracs(ch, max, clss);
      else
	PracSpell(ch, max, clss, arg);
      break;
    }

    return TRUE;
}

int GainLevel(struct char_data *ch, int clss)
{
    EXP temp;
    // how high do you go before you change recall point
    int max_newbie_level = 9;
    ubyte lvl;

    if (IS_NPC(ch)) {
        send_to_char("Perhaps you should return to your original form first.\n\r", ch);
	return (FALSE);
    }

    lvl = GET_LEVEL(ch, clss);

    if(lvl >= MAX_MORT)
    {
      send_to_char("Sorry but you have advanced to far for me to train you any further.\n\r",ch);
	return FALSE;
    }

/*
    if ((HowManyClasses(ch) > 1) && (lvl >= 50-10*(HowManyClasses(ch)-1)))
    {
      send_to_char("Sorry, but since you are so diverse, I cannot help you any farther.\n\r",ch);
      return FALSE;
    }
*/
    temp = exp_table[lvl];
    if (GET_EXP(ch) < temp)
    {
	send_to_char("You haven't got enough experience!\n\r",ch);
	return FALSE;
    }

    GET_EXP(ch) -= temp;

    send_to_char("You raise a level\n\r", ch);
    advance_level(ch, clss);
    set_title(ch);
    if (lvl > max_newbie_level && ch->player.hometown == 1800)
      ch->player.hometown = 7;

    return TRUE;
}

void ShowPracs(struct char_data* ch, int max, int clss)
{
    char buf[256];
    struct spell_info* spell;
    int told, i, splvl;
    int level = GET_LEVEL(ch, clss);

   if(!ch->skills) {
      cprintf(ch, "You don't have any skills/spells!\n\r");
      return;
   }

    sprintf(buf, "You have got %d practice sessions left.\n\r",
	    ch->specials.spells_to_learn);
    send_to_char(buf, ch);

   for(told=0, splvl=0; splvl <= MIN(level, max); splvl++) {
      for(i = 0, spell = spell_list ; i <= spell_count; i++, spell++) {
	 if(spell->name &&
	    (spell->targets & TAR_SKILL) &&
	    (spell->min_level[clss] == splvl)) {

	    if(spell->targets & TAR_PURE_CLASS && !IS_PURE_CLASS(ch))
		 continue;

	    if(!told){
	       told++;
	       send_to_char("You can practice the following skills:\n\r", ch);
	    }
	    sprintf(buf,"[%-3d]   $Cy%-25s$CN %s\n\r",
		    splvl, spell->name,
		    how_good(ch->skills[spell->number].learned));
	    send_to_char_formatted(buf, ch);
	 }
      }
   }
   if(told)
     send_to_char("\n\r", ch);

   for(splvl=0, told=0; splvl <= MIN(level, max); splvl++) {
      for(i = 0, spell = spell_list ; i <= spell_count; i++, spell++) {
	 if(spell->name &&
	    !(spell->targets & TAR_SKILL) &&
	    (spell->min_level[clss] == splvl)) {

	    if(spell->targets & TAR_PURE_CLASS && !IS_PURE_CLASS(ch))
	      continue;

	    if(!told) {
	       told++;
	       if (clss == BARD_LEVEL_IND) {
		  send_to_char("You can practice the following songs:\n\r", ch);
	       } else {
 		  send_to_char("You can practice the following spells:\n\r", ch);
	       }
	    }
	    sprintf(buf,"[%-3d]   $Cy%-25s$CN %s\n\r",
		    splvl, spell->name,
		    how_good(ch->skills[spell->number].learned));
	    send_to_char_formatted(buf, ch);
	 }
      }
   }
}

void PracSpell(struct char_data* ch, int max, int clss, char* arg)
{
    struct spell_info* spell;
    int percent, number;

    if(IS_NPC(ch))
    {
	send_to_char("I don't teach monsters.\n\r", ch);
	return;
    }

    if(!(spell = locate_spell(arg, FALSE)) ||
       (spell->min_level[clss] > max))
    {
	send_to_char("I don't know of that skill\n\r", ch);
	return;
    }

    if(spell->min_level[clss] > GET_LEVEL(ch, clss))
    {
	send_to_char("You don't know of that skill\n\r", ch);
	return;
    }

    if(ch->specials.spells_to_learn <= 0)
    {
	send_to_char("You don't seem to be able to practice now.\n\r", ch);
	return;
    }

    if((spell->targets & TAR_PURE_CLASS) && !IS_PURE_CLASS(ch)) {
       cprintf(ch, "You are too diversified to learn that skill.\n\r");
       return;
    }

    number = spell->number;

    max = LEARNED(spell);

    if(ch->skills[number].learned >= max)
    {
	send_to_char("You are already learned in this area.\n\r", ch);
	return;
    }

    send_to_char("You practice a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
    ch->skills[number].learned = MIN(max, percent);

    if (ch->skills[number].learned >= max)
	send_to_char("You are now learned in this area.\n\r", ch);
}

/*** the following special_proc is for the elf guards of the rivendell area.
     the routine processes all the chars in the room and sets the guard to
     attack the most evil aligned pc or npc. ***/

SPECIAL(attack_evil)
{
#ifndef NO_SMART_MOBS
    struct char_data *tempch, *evilch;
    struct char_data *guy = (struct char_data *) me;
    int evil_align=-350;

    if (type == SPEC_INIT)
	return (FALSE);

    /*** do nothing if asleep or fighting ***/
    if (cmd || !AWAKE(guy) || GET_POS(guy)==POSITION_FIGHTING)
	return (FALSE);

    /*** find the most evil pc or npc in the room (except for gods :) ***/
    evilch=NULL;
    for (tempch=real_roomp(guy->in_room)->people; tempch;
	 tempch=tempch->next_in_room)
    {
	if(IS_SET(tempch->specials.flags, PLR_PKILLER))
	{
	    if(!check_soundproof(guy))
		act ("$n screams 'Thou art a murderer and must be DESTROYED!'",
		    FALSE, guy, 0, 0, TO_ROOM);
	    hit(guy, tempch, TYPE_UNDEFINED);
	    return TRUE;
	}
	if (GET_ALIGNMENT(tempch)<=evil_align && (!IS_IMMORTAL(tempch)) && CAN_SEE(guy,tempch)) {
	    evil_align=GET_ALIGNMENT(tempch);
	    evilch=tempch;
	}
    }

    /*** if we found an evil pc or npc then attack it ***/
    if (evilch && (!check_peaceful(guy,""))) {
        if (GET_POS(guy)<POSITION_FIGHTING) {
            StandUp(guy);
            return (TRUE);
        }
	if (!check_soundproof(guy)) {

	    act ("$n screams 'Thy heart is EVIL! Thou must be DESTROYED!'",
		 FALSE, guy, NULL, evilch, TO_ROOM);
	}
	hit (guy, evilch, TYPE_UNDEFINED);
	return (TRUE);
    }

    /*** not fighting and no evil chars found ***/
    return (FALSE);
#else
    return (FALSE);
#endif
}


SPECIAL(possession_mob)
{
    struct char_data *tempch, *vict=NULL;
    struct char_data *zombie = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(zombie) || check_peaceful(zombie, ""))
        return (FALSE);

    if (zombie->specials.fighting) {
        vict=zombie->specials.fighting;
    } else {
        for (tempch=real_roomp(zombie->in_room)->people; tempch && !vict;
             tempch=tempch->next_in_room)
            if (CAN_SEE(zombie,tempch) && GET_RACE(tempch)==RACE_UNDEAD &&
               !IS_GOD(tempch) && zombie!=tempch)
                vict=tempch;
    }

    if (!vict)
       return (FALSE);

    if (GET_POS(zombie)<POSITION_FIGHTING) {
        StandUp(zombie);
        return (TRUE);
    }

    if (number(1,3)==1) /* 1 in 3 chance of not casting */
        return (FALSE);

    act ("$n laughs insanely and reaches out with a clawed black hand!",
        TRUE, zombie, 0, 0, TO_ROOM);
    cast_vampyric_touch (GetMaxLevel(zombie), zombie, "", SPELL_TYPE_SPELL, vict, 0);

    return (TRUE);
}


SPECIAL(fire_elemental)
{
    struct char_data *elemental = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(elemental) || !elemental->specials.fighting)
    return (FALSE);

    if (GET_POS(elemental)<POSITION_FIGHTING) {
	StandUp(elemental);
	return (TRUE);
    }

    act("$n draws upon its abilities and unleashes a lava storm!",
	TRUE,elemental,0,0,TO_ROOM);
    cast_lava_storm(GetMaxLevel(elemental), elemental, "", SPELL_TYPE_SPELL,
		    elemental->specials.fighting, 0);

    return (TRUE);
}


SPECIAL(water_elemental)
{
    struct char_data *elemental = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(elemental) || !elemental->specials.fighting)
	return (FALSE);

    if (GET_POS(elemental)<POSITION_FIGHTING) {
	StandUp(elemental);
	return (TRUE);
    }

    act("$n draws upon its abilities and unleashes a geyser!",
	TRUE,elemental,0,0,TO_ROOM);
    cast_geyser(GetMaxLevel(elemental), elemental, "", SPELL_TYPE_SPELL,
		elemental->specials.fighting, 0);

    return (TRUE);
}


SPECIAL(air_elemental)
{
    struct char_data *elemental = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(elemental) || !elemental->specials.fighting)
	return (FALSE);

    if (GET_POS(elemental)<POSITION_FIGHTING) {
	StandUp(elemental);
	return (TRUE);
    }

    act("$n draws upon its abilities and unleashes a windstorm!",
	TRUE,elemental,0,0,TO_ROOM);
    cast_wind_storm(GetMaxLevel(elemental), elemental, "", SPELL_TYPE_SPELL,
		    elemental->specials.fighting, 0);

    return (TRUE);
}


SPECIAL(earth_elemental)
{
    struct char_data *elemental = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd || !AWAKE(elemental) || !elemental->specials.fighting)
	return (FALSE);

    if (GET_POS(elemental)<POSITION_FIGHTING) {
	StandUp(elemental);
	return (TRUE);
    }

    act("$n draws upon its abilities and ulneashes and earthquake!",
	TRUE,elemental,0,0,TO_ROOM);
    cast_earthquake(GetMaxLevel(elemental), elemental, "", SPELL_TYPE_SPELL,
		    elemental->specials.fighting, 0);

    return (TRUE);
}

#define PRACTICE_COST 1000000
SPECIAL(practice_master)
{
    char buf[MAX_STRING_LENGTH];

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd!=56 && cmd!=59)
	return (FALSE);

    if (IS_NPC(ch)) {
	send_to_char("Try returning to your original form first.\n\r", ch);
	return (TRUE);
    }

    if (IS_IMMORTAL(ch)) {
	send_to_char("You are a god, I cannot help you.\n\r", ch);
	return (TRUE);
    }

    if (cmd==59) {
      send_to_char("I can show you methods with which you can expand your learning capacity.  For a mere 10 million experience points I can help you in this area.\n\r",ch);
	return(TRUE);
    }

    only_argument(arg, buf);
    if (!*buf || !is_abbrev(buf,"practices")) {
      send_to_char("I can only sell practices.",ch);
      return (TRUE);
    }

    if (GET_EXP(ch) < PRACTICE_COST)
    {
      send_to_char("You cannot afford my services.",ch);
      return(TRUE);
    }
    else if (ch->specials.spells_to_learn > 30)
    {
      send_to_char("You already have quite a few practices.  Come back when you have less.",ch);
      return(TRUE);
    }
    else
    {
      GET_EXP(ch)-=PRACTICE_COST;
      ch->specials.spells_to_learn++;
      send_to_char("Thank you for the experience.  You now have an extra practice. Use it well.",ch);
      return (TRUE);
    }
}


SPECIAL(erok)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *tmp_obj;
    static struct trade_stuff {
	char *name;
	int  clss;
	int  guild_rm;
    } trade[]=
    {
	{ "ma", CLASS_MAGIC_USER, 1687 },
	{ "cl", CLASS_CLERIC,     7922 },
	{ "wa", CLASS_WARRIOR,    5255 },
	{ "th", CLASS_THIEF,      5074 },
	{ "pa", CLASS_PALADIN,    25125 },
	{ "dr", CLASS_DRUID,      16769 },
	{ "ps", CLASS_PSI,        29652 },
	{ "ra", CLASS_RANGER,     6159 },
	{ 0,    0,                0 }
    };
    int x, num, cl, exp;
    struct trade_stuff *ptr;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd!=56 && cmd!=59)
	return (FALSE);

    if (IS_NPC(ch)) {
	send_to_char("Try returning to your original form first.\n\r", ch);
	return (TRUE);
    }

    cl=HowManyClasses(ch);
    if (cl==1) {
	send_to_char("You are already a single class character.\n\r", ch);
	return (TRUE);
    }

    if (IS_IMMORTAL(ch)) {
	send_to_char("You are a god, I cannot help you.\n\r", ch);
	return (TRUE);
    }

    if (cmd==59) {
	send_to_char("Please type one of the following to trade in...\n\r", ch);
	send_to_char("buy ma - trade in for a mage\n\r", ch);
	send_to_char("buy cl - trade in for a cleric\n\r",ch);
	send_to_char("buy wa - trade in for a warrior\n\r",ch);
	send_to_char("buy th - trade in for a thief\n\r",ch);
	send_to_char("buy pa - trade in for a paladin\n\r",ch);
	send_to_char("buy dr - trade in for a druid\n\r",ch);
	send_to_char("buy ps - trade in for a psionist\n\r",ch);
	send_to_char("buy ra - trade in for a ranger\n\r",ch);
	return(TRUE);
    }

    only_argument(arg, buf);
    if (!*buf) {
	send_to_char("Please specify which class you want to trade in for.\n\r", ch);
	return (TRUE);
    }

    num=0;
    for (ptr=trade; ptr->name; ptr++) {
	if (!strcmp(buf, ptr->name))
	    break;
	num++;
    }

    if (!ptr->name) {
	send_to_char("I don't recognize that class, type 'list' for help.\n\r", ch);
	return (TRUE);
    }

    command_interpreter(ch, "remove all", 1);
    act("Erok appeals to the Gods and you are transformed...",FALSE,ch,0,0,TO_CHAR);
    act("Erok appeals to the Gods and $n is transformed...",FALSE,ch,0,0,TO_ROOM);

    ch->player.clss=trade[num].clss; /* set class */
    exp = 0;
    for (x=0; x<MAX_LEVEL_IND; x++)  /* clear all levels in all classes */
    {
	if(GET_LEVEL(ch, x))
	    exp += total_exp(GET_LEVEL(ch, x));
	GET_LEVEL(ch, x)=0;
    }
    GET_LEVEL(ch, num)=1;	/* set level in chosen class */
    ch->points.max_hit=15;	/* everyone starts with 15 hp */
    ch->specials.spells_to_learn=7; /* everyone starts with 7 pracs */
    roll_abilities(ch);		/* reroll attributes */
    if (ch->skills) {		/* clear all skills */
	for (x=0; x<MAX_SKILLS; x++) {
	    ch->skills[x].learned=0;
	    ch->skills[x].recognise=FALSE;
	}
    } /* now give exp for level */
    GET_EXP(ch)=exp;

    act("Erok gives you a scroll of recall...",FALSE,ch,0,0,TO_CHAR);
    act("Erok gives $n a scroll of recall...",FALSE,ch,0,0,TO_ROOM);
    tmp_obj=make_object(3052, VIRTUAL);
    obj_to_char(tmp_obj, ch);

    act("Erok gestures and you are transferred to your guild...\n\r",FALSE,ch,0,0,TO_CHAR);
    act("Erok gestures and $n is transferred to $s guild.\n\r",FALSE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, trade[num].guild_rm);
    do_look(ch, "", 0);

    SaveChar(ch, AUTO_RENT, FALSE);
    send_to_char("\n\rErok tells you 'gain and practice in your new class now'\n\r",ch);

    return (TRUE);
}


SPECIAL(ClassMaster)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *tmp_obj;
    static struct trade_stuff {
	char *name;
        int level_ind;
	int  clss;
	int  guild_rm;
    } trade[]=
    {
	{ "bard", BARD_LEVEL_IND, CLASS_BARD, 3001 },
	{ "monk", MONK_LEVEL_IND, CLASS_MONK, 3001 },
	{ 0,     0,              0,          0 }
    };
    int x, num, cl;
    struct trade_stuff *ptr;
    struct char_data *mob;

    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd!=164) /* practice=164 */
	return  (FALSE);

    if (IS_NPC(ch)) {
	send_to_char("Try returning to your original form first.\n\r", ch);
	return (TRUE);
    }

    for (x=0; x<MAX_LEVEL_IND; x++)  /* clear all levels in all classes */
      if ( (HasClass(ch, 1 << x)) && (GET_LEVEL(ch, x) < 50) )
      {
	send_to_char("You are not yet ready to discuss such matters with me.\n\r", ch);
	return TRUE;
      }

    if (IS_IMMORTAL(ch)) {
	send_to_char("You are a god, I cannot help you.\n\r", ch);
	return (TRUE);
    }

    cl=HowManyClasses(ch);

    only_argument(arg, buf);

    if (!*buf) { /* tell them their options */
      send_to_char("You may practice in the following classes:\n\r", ch);
	send_to_char("  bard class\n\r", ch);
      if (cl > 1) /* multi's only */
        send_to_char("  monk class\n\r", ch);
	return(TRUE);
    }

    num=0;
    for (ptr=trade; ptr->name; ptr++) {
	if (!strcmp(buf, ptr->name))
	    break;
	num++;
    }

    if (!ptr->name) {
	send_to_char("I don't recognize that class, type 'practice' for a listing.\n\r", ch);
	return (TRUE);
    }

    command_interpreter(ch, "remove all", 1);
    act("The Classmaster appeals to the Gods and you are transformed...",FALSE,ch,0,0,TO_CHAR);
    act("The Classmaster appeals to the Gods and $n is transformed...",FALSE,ch,0,0,TO_ROOM);

    ch->player.clss=trade[num].clss; /* set class */
    for (x=0; x<MAX_LEVEL_IND; x++)  /* clear all levels in all classes */
    {
      ch->player.max_level[x]=0;
      GET_LEVEL(ch, x)=0;
    }
    GET_LEVEL(ch, trade[num].level_ind)=1;	/* set level in chosen class */
    UpdateMaxLevel(ch);
    UpdateMinLevel(ch);
    if (ch->skills) {		/* set all skills learned to 15 percent */
      for (x=0; x<MAX_SKILLS; x++) {
	if (ch->skills[x].learned)
	  ch->skills[x].learned=15;
	else
	{
	  ch->skills[x].learned=0;
	  ch->skills[x].recognise=FALSE;
	}
      }
    } /* now give 1 exp so they can't gain at fountain */
    GET_EXP(ch)=1;

    act("The Classmaster gives you a scroll of recall...",FALSE,ch,0,0,TO_CHAR);
    act("The Classmaster gives $n a scroll of recall...",FALSE,ch,0,0,TO_ROOM);
    tmp_obj=make_object(3052, VIRTUAL);
    obj_to_char(tmp_obj, ch);

    mob = (struct char_data *) me;

    do_say(mob, "It is a hard road upon which you have chosen to set your feet.",17);
    do_say(mob, "You must give up what you have learned in the past in order to have any hope of suceeding in this new endeavor.", 17);
    do_say(mob, "Go forth now and study diligently!", 17);

    act("The Classmaster gestures and you are transferred to your guild...\n\r",FALSE,ch,0,0,TO_CHAR);
    act("The Classmaster gestures and $n is transferred to $s guild.\n\r",FALSE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, trade[num].guild_rm);
    do_look(ch, "", 0);

    SaveChar(ch, AUTO_RENT, FALSE);
    send_to_char("\n\rThe Classmaster tells you 'gain and practice in your new class now'\n\r",ch);

    return (TRUE);
}


/*
 * a bounty hunter process to hunt down characters with a pkill bit set
 */
SPECIAL(bounty_hunter)
{
  struct char_data *i;
  struct char_data *found;
  struct char_data *hunter = (struct char_data *) me;
  int level, my_lev = GetMaxLevel(hunter);

  if (type == SPEC_IDLE)
  {
      if(attack_convict(hunter))	/* if we've found one */
      {
	  if(hunter->hunt_info)		/* cancel any in progress track */
	  {
	      path_kill(hunter->hunt_info);
	      hunter->hunt_info = 0;
	  }
	  return TRUE;
      }

      if (!hunter->hunt_info)		/* if we don't have a target */
      {
	  found = NULL;
	  level = 0;

	  EACH_CHARACTER(iter, i)	/* find a pkiller below our level */
	  {
	      if(IS_PC(i) &&
		 IS_SET(i->specials.flags, PLR_PKILLER) &&
		 (GetMaxLevel(i) <= my_lev))
	      {
		  SetHunting(hunter, i);	/* try to set them hunting */
		  if(hunter->hunt_info)
		  {
		      act("You go in search of $N",
			  TRUE, hunter, NULL, i, TO_CHAR);
		      act("$n gets a determined look on his face",
			  TRUE, hunter, NULL, i, TO_ROOM);
		      break;
		  }
	      }
	  }
	  END_AITER(iter);

	  if(hunter->hunt_info)
	      return TRUE;
      }
      else
      {
	  /* check to make sure that the victim still has a pkill bit */
	  if(!IS_SET(hunter->hunt_info->victim->specials.flags, PLR_PKILLER))
	  {
	      act("You sense $N has changed their evil ways",
		  TRUE, hunter, NULL, hunter->hunt_info->victim, TO_CHAR);
	      act("$n spits in disgust.",
		  TRUE, hunter, NULL, NULL, TO_ROOM);
	      path_kill(hunter->hunt_info);
	      hunter->hunt_info = 0;
	  }
      }
  }

  return alkian_warrior(me, ch, 0, "", type);
}


int attack_convict(struct char_data* ch)
{
    struct char_data* targ;
    struct room_data* rp;

    if((rp = real_roomp(ch->in_room)) == NULL)
	return 0;

    for(targ = rp->people ; targ ; targ = targ->next_in_room)
    {
	if(IS_SET(targ->specials.flags, PLR_PKILLER))
	{
	    act("$n screams 'Murder! Scum of the earth!'",
		FALSE, ch, NULL, targ, TO_ROOM);
	    hit(ch, targ, TYPE_UNDEFINED);
	    return TRUE;
	}
    }

    return FALSE;
}


SPECIAL(liquid_proc)
{
    char buf[MAX_INPUT_LENGTH];
    struct char_data *pool = (struct char_data *) me;
    static int legal[]={ 1, 2, 3, 4, 5, 6, 7 /*dirs and enter*/, 8 /*exits*/,
	16 /*score*/, 38 /*help*/, 39 /*who*/, 42/*stand*/, 43/*sit*/,
	54/*news*/, 62/*weather*/, 69/*save*/, 76 /*time*/, 168
/*info*/,
        174 /*levels*/, 199 /*brief*/, 200 /*wizlist*/, 201 /*consider*/,
        212 /*credits*/, 213 /*compact*/, 215 /*deafen*/,  236 /*attribute*/,
        237 /*world*/, 238 /*allspells*/, 259 /*spells*/, 320 /*commands*/,
        337 /*display*/, 341 /*color*/, 347 /*notell*/, 348 /*flags*/,
        350 /*autoexit*/, 351 /*helptopics*/, 400 /*motd*/, 404 /*page*/,
        417 /*channels*/, 234 /*whozone*/, 41 /*echo*/, 204 /*return*/,
        15 /*look*/, 253 /*leech*/, 0
    };
    int x;

    if (type == SPEC_INIT)
	return (FALSE);

    if (!cmd)
	return FALSE;

    if (pool == ch)
    {
	for (x = 0; legal[x]; x++)
	    if (legal[x] == cmd)
	    {
		act("You notice some strange motions in $n...",
		    TRUE, pool, 0, 0, TO_ROOM);
		return FALSE;
	    }

	send_to_char("You can't do much of anything in this form.\n\r", pool);
	return TRUE;
    }

    else
    {
	one_argument(arg, buf);

	if ((cmd == 15 || cmd == 166) &&
	    (pool == get_char_room_vis(ch, buf))) {  /* intercept looks */
	    send_to_char("The pool seems almost alive as the murky "
			 "liquid swirls about.\n\r", ch);
	    act("$n looks curiously at $N.", TRUE, ch, 0, pool, TO_ROOM);
	    act("$n looks curiously at you.", TRUE, ch, 0, pool, TO_VICT);
	    return (TRUE);
	}

	return (FALSE);
    }

    return FALSE;
}


SPECIAL(cocoon_proc)
{
    struct char_data *cocoon = (struct char_data *) me;
    char buf[MAX_INPUT_LENGTH];
    static int legal[]={ 204 /*return*/,
        16 /*score*/, 38 /*help*/, 39 /*who*/,  54 /*news*/,
	69 /*save*/, 76 /*time*/, 168 /*info*/,  174 /*levels*/,
        200 /*wizlist*/, 212 /*credits*/, 215 /*deafen*/,  236 /*attribute*/,
        237 /*world*/, 238 /*allspells*/,
        259 /*spells*/, 320 /*commands*/, 341 /*color*/,
        347 /*notell*/, 348 /*flags*/, 351 /*helptopics*/,
	400 /*motd*/, 404 /*page*/, 417 /*channels*/, 234 /*whozone*/,
	41 /*echo*/, 0
    };
    int x;

    if (type == SPEC_INIT)
	return (FALSE);

    if (!cmd)
    {
	GET_HIT(cocoon) = MIN(GET_HIT(cocoon) + hit_gain(cocoon),
			      GET_MAX_HIT(cocoon));
	GET_MANA(cocoon) = MIN(GET_MANA(cocoon) + mana_gain(cocoon),
			      GET_MAX_MANA(cocoon));
	GET_MOVE(cocoon) = MIN(GET_MOVE(cocoon) + move_gain(cocoon),
			       GET_MAX_MOVE(cocoon));
	send_to_char("You regenerate...\n\r", cocoon);
	return (TRUE);
    }


    if (ch == cocoon)
    {
	for (x = 0; legal[x]; x++)
	    if (legal[x] == cmd)
	    {
		act("You notice some movement from within $n...",
		    TRUE, cocoon, 0, 0, TO_ROOM);
		return (FALSE);
	    }

	send_to_char("You can't do much in this regenerative state.\n\r",
		     cocoon);
	return TRUE;
    }

    else
    {
	one_argument(arg, buf);

	if ((cmd == 15 || cmd == 166) &&
	    (cocoon == get_char_room_vis(ch, buf))) {
	    send_to_char("The cocoon pulsates slowly, and you notice "
			 "movement through its opaque skin.\n\r", ch);
	    act("$n examines $N.", TRUE, ch, 0, cocoon, TO_ROOM);
	    act("$n examines you closely.", TRUE, ch, 0, cocoon, TO_VICT);
	    return TRUE;
	}

	return FALSE;
    }

    return FALSE;
}


/*  THIS BLOCKER CODE BELOW HAS BEEN REPLACED BY THE BLOCK.C CODE! */
/*  THIS CODE ISNT CALLED FROM THE SPEC_ASSIGN.C FILE */
/*  THE VNUMS ON THE MONSTERS AND ROOMS ARE MOST LIKELY INVALID */
typedef struct
{
    char*	vlook;
    char*	olook;
    char*	vsilent;
    char*	osilent;
    char*	vnoisy;
    char*	onoisy;
} BlockMesg;

enum block_type
{
    blockEnd,
    blockAll,
    blockLevel,
    blockClass
};

typedef struct
{
    room_num   	room;
    int		mob;
    int		dirs;
    block_type  type;
    long	flags;
    BlockMesg*	mesgs;
} BlockInfo;

BlockMesg StdMsg =
{
    NULL,
    NULL,
    "$n blocks your way.",
    "$n blocks $N's way.",
    "$n blocks your way.",
    "$n blocks $N's way."
};

BlockMesg SisyMsg =
{
    "$n looks at you.",
    "$n looks at $N.",
    "$n grins evilly.",
    "$n grins evilly.",
    "$n says, 'You'll, never get by me alive.'",
    "$n says, 'You'll, never get by me alive.'",
};

BlockInfo Blockers[] =
{
/* new thalos guild guards */
    { 13532,  3644, 2,	blockClass,	CLASS_THIEF,		&StdMsg },
    { 13512,  3657, 2,	blockClass,	CLASS_CLERIC,		&StdMsg },
    { 13526,  3633, 2,	blockClass,	CLASS_WARRIOR,		&StdMsg },
    { 13525,  3656, 0,	blockClass,	CLASS_MAGIC_USER,	&StdMsg },

/* mordilnia guild guards */
    { 18266, 18210, 3,	blockClass,	CLASS_MAGIC_USER,	&StdMsg },
    { 18276, 18211, 1,	blockClass,	CLASS_CLERIC,		&StdMsg },
    { 18272, 18212, 0,	blockClass,	CLASS_THIEF,		&StdMsg },
    { 18256, 18213, 2,	blockClass,	CLASS_WARRIOR,		&StdMsg },
    { 18189, 18206, 4,	blockClass,	CLASS_PALADIN,		&StdMsg },
    { 18191, 18207, 2,	blockClass,	CLASS_RANGER,		&StdMsg },
    { 18193, 18208, 2,	blockClass,	CLASS_DRUID,		&StdMsg },
    { 18195, 18209, 5,	blockClass,	CLASS_PSI,		&StdMsg },

/* high guild guards */
    {  1686, 25139, 5,	blockClass,	CLASS_MAGIC_USER,	&StdMsg },
    {  7921, 25142, 4,	blockClass,	CLASS_CLERIC,		&StdMsg },
    {  5073, 25141, 5,	blockClass,	CLASS_THIEF,		&StdMsg },
    {  5254, 25140, 2,	blockClass,	CLASS_WARRIOR,		&StdMsg },
    { 25124, 25143, 5,	blockClass,	CLASS_PALADIN,		&StdMsg },
    {  6158, 25144, 5,	blockClass,	CLASS_RANGER,		&StdMsg },
    { 16768, 25145, 5,	blockClass,	CLASS_DRUID,		&StdMsg },
    { 29561, 25146, 5,	blockClass,	CLASS_PSI,		&StdMsg },

/* caravan guild guards */
    { 16115, 16108, 1,	blockClass,	CLASS_MAGIC_USER,	&StdMsg },
    { 16126, 16110, 1,	blockClass,	CLASS_CLERIC,		&StdMsg },
    { 16117, 16109, 3,	blockClass,	CLASS_THIEF,		&StdMsg },
    { 16110, 16107, 3,	blockClass,	CLASS_WARRIOR,		&StdMsg  },

/* sisyphus */
    {  1499,  1499, 3,	blockLevel,	-(SISYPHUS_MAX_LEVEL+1),&SisyMsg },

/* end marker, must be present to win */
    {	  0,     0, 0,	blockEnd,	0,			NULL }
};


SPECIAL(Blocker)
{
    BlockInfo*		block;
    struct room_data*	room;
    struct char_data*	blocker = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if(cmd > 6)			/* not a movement command we don't care */
	return FALSE;

				/* look to see if this room has a blocker */
    for(block = Blockers ; block->type != blockEnd ; block++)
	if(block->room == ch->in_room)
	    break;

    if(block->type == blockEnd)	/* blocker doesn't belong in this room */
	return FALSE;

    if((cmd - 1) != block->dirs) /* blocker doesn't watch this direction */
	return FALSE;

    if(!(room = real_roomp(ch->in_room))) /* can't find this room */
	return FALSE;

    if(block->mesgs->vlook)
	act(block->mesgs->vlook, TRUE, blocker, NULL, ch, TO_VICT);
    if(block->mesgs->olook)
	act(block->mesgs->olook, TRUE, blocker, NULL, ch, TO_ROOM);

    switch(block->type)
    {
    case blockAll:
	break;

    case blockLevel:
	if(block->flags < 0)
	{
	    if(GetMaxLevel(ch) < -block->flags)
		return FALSE;
	}
	else if(GetMaxLevel(ch) > block->flags)
	    return FALSE;
	break;

    case blockClass:
	if(block->flags & ch->player.clss)
	    return FALSE;
	break;

    default:
	break;
    }

    if(check_soundproof(blocker))
    {
	if(block->mesgs->vsilent)
	    act(block->mesgs->vsilent, TRUE, blocker, NULL, ch, TO_VICT);
	if(block->mesgs->osilent)
	    act(block->mesgs->osilent, TRUE, blocker, NULL, ch, TO_ROOM);
    }
    else
    {
	if(block->mesgs->vnoisy)
	    act(block->mesgs->vnoisy, TRUE, blocker, NULL, ch, TO_VICT);
	if(block->mesgs->onoisy)
	    act(block->mesgs->onoisy, TRUE, blocker, NULL, ch, TO_ROOM);
    }

    return TRUE;
}


SPECIAL(mob_hero_ring)
{
    int action=0;
    int rl,i;
    struct obj_data *obj, *ring;
    struct char_data *mobile = (struct char_data *) me;
    struct char_data *victim;
    char buf[MAX_STRING_LENGTH];

    if (type == SPEC_INIT)
      return (FALSE);

    if(!cmd)
	return(FALSE);

    if ((rl=real_object(HERO_RING))==-1)
    {
	log_msg("The Hero Ring is missing!", LOG_MPROG);
	return(FALSE);
    }

    if (cmd!=98)
	return(FALSE);
    else
    /* if bowing */
    {
	if(IS_NPC(ch))
	    return(FALSE);

	if (!heroes[0].name)
	    action=0;
	else
	  if (is_top_hero(ch))
	  {
	    action=2;
	    for(i=1;i<=obj_index[rl].number;i++)
	    {
	      /* where is this ring? */
	      sprintf(buf, "%d.hero ring.", i);
	      if (NULL== (ring=get_obj_vis_world(ch, buf, NULL)))
		/* there aren't as many hero rings in the game as we thought */
	        break;
	      else
		if( (victim=char_holding(ring)) != NULL)
		{
		  if (ch==victim)
		    action=0;
		}
	    }
	  }

	switch(action)
	{
	case 0: /* default, not hero  */
	    act("As you bow, $N smiles to you.",TRUE,ch,NULL,mobile,TO_CHAR);
	    act("As $N bows, you smile to $M.",TRUE,mobile,NULL,ch,TO_CHAR);
	    act("$N smiles as $n bows to $M.",TRUE,ch,NULL,mobile,TO_ROOM);
	    sprintf(buf,"%s, %s",GET_REAL_NAME(ch),"I am looking for the hero of the land. Have you seen such a one?");
	    do_say(mobile,buf,0);
	    break;
	case 2:
	    if(!(obj=make_object(rl,REAL|NORAND)))
	       return(FALSE);
	    act("As you bow, $N smiles and bestows upon you the $p.",TRUE,ch,obj,mobile,TO_CHAR);
	    act("As $N bows, you smile and hand over the $p.",TRUE,mobile,obj,ch,TO_CHAR);
	    act("As $n bows, $N smiles and hands the $p to $m.",TRUE,ch,obj,mobile,TO_ROOM);
	    obj_to_char(obj,ch);
	    break;
	}
    }
    return(TRUE);
}


int herohunter_go(struct char_data *ch,room_num rnum)
{
  unsigned long rmflags;

  rmflags=RM_FLAGS(rnum);

  if(ch->in_room==rnum)
    return FALSE;

  if( IS_SET(rmflags,GOD_RM) || IS_SET(rmflags,IMMORT_RM) )
    return FALSE;
  else
  {
     act("The $n dematerializes.",FALSE,ch,NULL,NULL,TO_ROOM);
     char_from_room(ch);
     char_to_room(ch,rnum);
     act("The $n materializes in the room.",FALSE,ch,NULL,NULL,TO_ROOM);
     return TRUE;
   }
}


int herohunter_take(struct char_data *ch,
		    struct obj_data *obj, struct char_data *victim)
{
  if((!obj)||(!ch)||(!victim))
    return(FALSE);

  if (obj->carried_by)
    obj_from_char(obj);
  else if (obj->equipped_by)
    unequip_char(victim, obj->eq_pos);
  else if (obj->in_obj)
    obj_from_obj(obj);
  obj_to_char(obj, ch);

  return TRUE;
}


SPECIAL(herohunter)
{
    static char buf[256];
    struct char_data *victim;
    int i;
    static int nr=-1;
    static int	howmanyrings=-1;
    struct obj_data	*ring=NULL;
    struct wraith_hunt {
	int	ringnumber;
	int	chances;
    } *wh;
    room_num rnum;
    struct char_data *hunter = (struct char_data *) me;

    if (type == SPEC_INIT)
	return (FALSE);

    if ( !IS_NPC(hunter) || cmd) {
	return(FALSE);
    }

    if(GET_POS(hunter)<POSITION_STANDING)
    {
      do_wake(hunter,"",0);
      do_stand(hunter,"",0);
      return(TRUE);
    }

    if (nr==-1)
      nr=real_object(HERO_RING);

    if (hunter->specials.fighting) {
	generic_warrior(me, ch, cmd, arg, type);
	return(FALSE);
    }

    /* does our ringwraith have his state info? */
    if (hunter->act_ptr==0) {
	CREATE(wh, struct wraith_hunt, 1);
	hunter->act_ptr = (int) wh;
	wh->ringnumber=0;
    } else
	wh = (struct wraith_hunt*)hunter->act_ptr;
        /* yuck, talk to loki about the act_ptr */

    /* how many hero rings are in the game? */
    if ( (howmanyrings=obj_index[nr].number) <1) {
      /* no hero rings available to search for */
      wh->ringnumber=0;
      wh->chances=0;
      return(herohunter_go(hunter,HEROHUNTER_HOME));
    }

    /* is he currently tracking a ring */
    if (!wh->ringnumber) {
      wh->chances=0;
      wh->ringnumber = 0;

      for(i=1;i<=howmanyrings;i++)
      {
	/* where is this ring? */
	sprintf(buf, "%d.hero ring.", i);
	if (NULL== (ring=get_obj_vis_world(hunter, buf, NULL))) {
	  /* there aren't as many hero rings in the game as we thought */
	  return FALSE;
	} else {
	  if( (victim=char_holding(ring)) != NULL)
	  {
	    if( is_top_hero( victim ) )
	    {
	      /* dude carrying ring is a hero, so skip it */
	      ring=NULL;
	      continue;
	    }
	  }
	  rnum = room_of_object(ring);

	  if (rnum != hunter->in_room)
	  {
	    wh->ringnumber=i;
	    return(herohunter_go(hunter,rnum));
	  }
	}

	if(ring)
	{
	  /* if we got this far, then we have found a ring that needs
	   * to be collected!.
	   */
	  wh->ringnumber=i;
	  wh->chances=0;
	  break;
	}
      }
    }

    if( (wh->ringnumber) && (!ring) )
      {
	sprintf(buf, "%d.hero ring.", wh->ringnumber);
	if (NULL== (ring=get_obj_vis_world(hunter, buf, NULL))) {
	  wh->ringnumber=0;
	  return(herohunter_go(hunter,HEROHUNTER_HOME));
	}
	victim=char_holding(ring);
      }
    else
      return FALSE;

    if(victim)
      if(is_top_hero(victim))
      {
	wh->ringnumber=0;
	return FALSE;
      }

    if( (rnum = room_of_object(ring)) !=hunter->in_room)
      return(herohunter_go(hunter,rnum));

    /* the ring is in the same room! */

    if (victim) {
	if (victim==hunter) {
	    obj_from_char(ring);
	    extract_obj(ring);
	    wh->ringnumber=0;
	    act("$n grimaces happily.", FALSE, hunter, NULL, victim, TO_ROOM);
	} else {
	    switch (wh->chances) {

	    case 0:
		do_wake(hunter, GET_NAME(victim), 0);
		if (!check_soundproof(hunter))
		{
		  act("$n says '$N, give me the $p'.", FALSE, hunter, ring, victim,
		      TO_VICT);
		  act("$n says '$N, give me the $p'.", FALSE, hunter, ring, victim,
		      TO_ROOM);
		}
		else
		{
		  act("$n pokes you in the ribs.", FALSE, hunter, NULL, victim,
		      TO_VICT);
		  act("$n pokes $N in the ribs.", FALSE, hunter, NULL, victim,
		      TO_ROOM);
		}

		wh->chances++;
		return(TRUE);
		break;

	      case 1:
		if (IS_NPC(victim)) {
		    act("$N quickly surrenders the $p to $n.", FALSE, hunter, ring,
			victim,	TO_ROOM);
		    return(herohunter_take(hunter,ring,victim));
		} else {
		  if (!check_soundproof(hunter))
		  {
		    act("$n says '$N, give me the $p *NOW*'.",
			    FALSE, hunter, ring, victim,
			    TO_VICT);
		    act("$n says '$N, give me the $p *NOW*'.",
			    FALSE, hunter, ring, victim,
			    TO_ROOM);
		  } else {
		    act("$n pokes $N in the ribs very painfully.",
			    FALSE, hunter, NULL, victim, TO_ROOM);
		    act("$n pokes you in the ribs very painfully.",
			    FALSE, hunter, NULL, victim, TO_VICT);
		    }
		    wh->chances++;
		}
		return(TRUE);
		break;

	      default:
		act("Something powerful overwhelmes you...\n\r"
		    "You surrender the $p to $n.", FALSE, hunter, ring,
			victim,	TO_VICT);
		act("$N quickly surrenders the $p to $n.", FALSE, hunter, ring,
			victim,	TO_ROOM);
		return(herohunter_take(hunter,ring,victim));
	    }
	}
    } else if (ring->in_obj) {
	/* the ring is in an object */
	act("$n gets the $p from the $P.", FALSE, hunter, ring, ring->in_obj,
	    TO_ROOM);
	obj_from_obj(ring);
	obj_to_char(ring, hunter);
	return TRUE;
    } else if (ring->in_room != NOWHERE) {
	obj_from_room(ring);
	obj_to_char(ring, hunter);
	act("$n gets the $p.", FALSE, hunter, ring, 0, TO_ROOM);
	return TRUE;
    } else {
	log_msg("a Hero Ring was completely disconnected!?", LOG_MPROG);
	wh->ringnumber = 0;
    }
    return FALSE;
}


SPECIAL(banana)
{
    if (type == SPEC_INIT)
	return (FALSE);

    if (!cmd) return(FALSE);

    if ((cmd >= 1) && (cmd <= 6) &&
	(GET_POS(ch) == POSITION_STANDING) &&
	(!IS_NPC(ch))) {
	if (!saves_spell(ch, SAVING_PARA, 0)) {
	    act("$N tries to leave, but slips on a banana and falls.",
		TRUE, ch, 0, ch, TO_NOTVICT);
	    act("As you try to leave, you slip on a banana.",
		TRUE, ch, 0, ch, TO_VICT);
	    GET_POS(ch) = POSITION_SITTING;
	    return(TRUE); /* stuck */
	}
	return(FALSE);	/* he got away */
    }

    return(FALSE);
}


SPECIAL(BirdKing)
{
    int variouscrap=0;
    room_num target;
    struct char_data* temp = NULL;
    struct room_data* room= NULL;
    static int fight[][MAX_SET_SIZE] = {
	{SPELL_CREEPING_DOOM, SPELL_LAVA_STORM, SPELL_ICE_STORM,
	 SPELL_ACID_RAIN, SPELL_POISON_GAS, SPELL_CHAIN_ELECTROCUTION, 0},
	{SPELL_DISPEL_MAGIC, SKILL_STUN , 0},
	{SPELL_FROST_CLOUD, SPELL_ACID_BLAST, SPELL_FIREBALL,
	 SPELL_ELECTRIC_FIRE, SPELL_VAMPYRIC_TOUCH, SPELL_DISINTEGRATE,
	 SKILL_PSIONIC_BLAST, 0},
	{0}
    };
    static int peace[][MAX_SET_SIZE] = {
	{SPELL_SANCTUARY, SPELL_FIRESHIELD, SPELL_HEAL, 0},
	{SPELL_INVISIBLE, 0},
	{SKILL_HIDE, 0},
	{0},
    };

    struct char_data *king = (struct char_data *) me;

    target = king->in_room;

    switch (type)
    {
    case SPEC_INIT:
	init_atts(king, 200, 200);
	init_skills(king, DRUID_LEVEL_IND);
	init_skills(king, MAGE_LEVEL_IND);
	init_skills(king, PSI_LEVEL_IND);
	init_skills(king, CLERIC_LEVEL_IND);
	init_skills(king, RANGER_LEVEL_IND);
       	return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
	do_abilities(king, fight, peace, cmd);
	room = real_roomp(3399);  /*elite guard room*/

	if (!(room->people))
	{
	    /* Hey - no ones in the room. wouldn't want to call for them */
	    return (TRUE);
	}
	if (king->specials.fighting)
	{
	    room = real_roomp(3399);  /*elite guard room*/
	    if (!(room->people)) {
		/* Hey - no ones in the room. wouldn't want to call for them */
		return (TRUE);
	    }

	    act("$n yells out 'GUARDS! TO ME!'", FALSE, king, 0, 0, TO_ROOM);

	    if (!room) {
		act("$n yells out, 'Where are my damn guards!?!?'",
		    FALSE, king, 0, 0, TO_ROOM);
		return (FALSE);
	    }

	    act("An elite guard enters the hall and attacks!",
		TRUE, ch, 0, 0, TO_CHAR);

	    temp = room->people;

	    for (temp = room->people; temp; temp = room->people)
	    {
		variouscrap++;
		if (variouscrap > 10)
		    return (FALSE);

		if (target == room->number)
		    return (FALSE);

		char_from_room(temp);
		char_to_room(temp, target);
		set_fighting(ch, temp);
		if (!temp)
		    return (FALSE);
		room = real_roomp(3399);  /*elite guard room*/

	    }
	}
	return (TRUE);
    }

    return (FALSE);
}

/*  More stuff added by Novak for  Alkian Burial Grounds */

SPECIAL(alkian_warrior)
{
  struct char_data *warrior = (struct char_data *) me;

  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK,  0},
    {SPELL_VAMPYRIC_TOUCH, SPELL_DISINTEGRATE, 0},
    {SKILL_BASH, SKILL_KICK, SKILL_DODGE, 0},
    {SKILL_STUN, SPELL_LAVA_STORM, SPELL_ACID_RAIN, 0},
    {SKILL_PSIONIC_BLAST, SKILL_THOUGHT_THROW, 0},
    {SPELL_HEAL,SKILL_FLAIL, 0},
    {0}
  };
  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_HEAL, SPELL_FIRESHIELD, 0},
    {SPELL_SHIELD, SPELL_ARMOR, SPELL_STONE_SKIN, 0},
    {SPELL_ELECSHIELD, SPELL_COLDSHIELD, SPELL_POISONSHIELD, 0},
    {SKILL_VAMPSHIELD, SKILL_MANASHIELD, SPELL_MOVESHIELD, 0},
    {SPELL_BLESS, 0},
    {0}
  };

  switch(type)
    {
    case SPEC_INIT:
      init_atts(warrior, 500, 500);
      init_skills(warrior, MAGE_LEVEL_IND);
      init_skills(warrior, PSI_LEVEL_IND);
      init_skills(warrior, CLERIC_LEVEL_IND);
      init_skills(warrior, RANGER_LEVEL_IND);
      init_skills(warrior, WARRIOR_LEVEL_IND);
      init_skills(warrior, DRUID_LEVEL_IND);
      return(TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(warrior, fight, peace, cmd));
    }

  return(FALSE);
}

SPECIAL(alkian_fetch)
{
  struct char_data *fetch = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK, SKILL_DODGE, 0},
    {SPELL_VAMPYRIC_TOUCH, SKILL_BASH, SPELL_FIREBALL, SKILL_STUN, 0},
    {SPELL_LAVA_STORM, SPELL_DISINTEGRATE, SPELL_ACID_RAIN, 0},
    {SPELL_HEAL, SPELL_ACID_BLAST, 0},
    {0}
  };
  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_HEAL, SPELL_FIRESHIELD, 0},
    {SPELL_SHIELD, SPELL_ARMOR, SPELL_STONE_SKIN, 0},
    {0}
  };

  switch(type)
    {
    case SPEC_INIT:
      init_atts(fetch, 500, 500);
      init_skills(fetch, MAGE_LEVEL_IND);
      init_skills(fetch, PSI_LEVEL_IND);
      init_skills(fetch, CLERIC_LEVEL_IND);
      init_skills(fetch, RANGER_LEVEL_IND);
      init_skills(fetch, WARRIOR_LEVEL_IND);
      init_skills(fetch, DRUID_LEVEL_IND);
      return(TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(fetch, fight, peace, cmd));
    }

  return(FALSE);
}

SPECIAL(alkian_holly)
{
  struct char_data *holly = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK, SKILL_BASH, SKILL_LAY_ON_HANDS, 0},
    {SPELL_DISINTEGRATE, SPELL_FIREBALL, SPELL_LAVA_STORM, SKILL_STUN, 0},
    {0}
  };

  static int peace[][MAX_SET_SIZE] = {
    {SKILL_BLESSING, SPELL_HEAL, SPELL_SANCTUARY, SPELL_FIRESHIELD, 0},
    {SKILL_MEDITATE, 0},
    {0}
  };

  switch (type)
    {
    case SPEC_INIT:
      init_atts(holly, 500, 500);
      init_skills(holly, CLERIC_LEVEL_IND);
      init_skills(holly, PALADIN_LEVEL_IND);
      init_skills(holly, MAGE_LEVEL_IND);
      init_skills(holly, PSI_LEVEL_IND);
      init_skills(holly, WARRIOR_LEVEL_IND);
      init_skills(holly, RANGER_LEVEL_IND);
      return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(holly, fight, peace, cmd));
    }

  return (FALSE);
}


SPECIAL(alkian_demonspawn)
{
  struct char_data *demonspawn = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK, SKILL_DODGE, 0},
    {SPELL_VAMPYRIC_TOUCH, SPELL_FIREBALL, SKILL_STUN, 0},
    {SPELL_EARTHQUAKE, SPELL_CALL_LIGHTNING, SPELL_CREEPING_DOOM, 0},
    {SPELL_LAVA_STORM, SPELL_DISINTEGRATE, SPELL_ACID_RAIN, 0},
    {SPELL_HEAL, SPELL_ACID_BLAST, 0},
    {SPELL_POISON, SPELL_POISON_GAS, 0},
    {0}
  };

  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_HEAL, SPELL_FIRESHIELD, 0},
    {SPELL_SHIELD, SPELL_ARMOR, SPELL_STONE_SKIN, 0},
    {0}
  };

  switch(type)
    {
    case SPEC_INIT:
      init_atts(demonspawn, 500, 500);
      init_skills(demonspawn, MAGE_LEVEL_IND);
      init_skills(demonspawn, PSI_LEVEL_IND);
      init_skills(demonspawn, CLERIC_LEVEL_IND);
      init_skills(demonspawn, RANGER_LEVEL_IND);
      init_skills(demonspawn, WARRIOR_LEVEL_IND);
      init_skills(demonspawn, DRUID_LEVEL_IND);
      return(TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(demonspawn, fight, peace, cmd));
    }

  return(FALSE);
}

SPECIAL(alkian_undertaker)
{
  struct char_data *undertaker = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_EARTHQUAKE, SPELL_CREEPING_DOOM, SPELL_VINE, SPELL_THORN, 0},
    {SPELL_CALL_LIGHTNING, SPELL_HEAL, SKILL_BERSERK, SKILL_STUN, 0},
    {0}
  };

  static int peace[][MAX_SET_SIZE] = {
    {SPELL_HEAL, SPELL_SANCTUARY, SPELL_PETRIFY, SPELL_FIRESHIELD, 0},
    {SPELL_SHIELD, SPELL_ARMOR, 0},
    {0}
  };

  switch (type)
    {
    case SPEC_INIT:
      init_atts(undertaker, 500, 500);
      init_skills(undertaker, CLERIC_LEVEL_IND);
      init_skills(undertaker, MAGE_LEVEL_IND);
      init_skills(undertaker, DRUID_LEVEL_IND);
      init_skills(undertaker, RANGER_LEVEL_IND);
      return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(undertaker, fight, peace, cmd));
    }

  return (FALSE);
}

SPECIAL(generic_alkian)
{
  struct char_data *alkian = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK, 0},
    {SPELL_VAMPYRIC_TOUCH, SPELL_HEAL, 0},
    {SKILL_STUN, SPELL_POISON, 0},
    {SKILL_PSIONIC_BLAST, 0},
    {0}
  };

  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_FIRESHIELD, SPELL_HEAL, 0},
    {SPELL_ARMOR, SPELL_SHIELD, SPELL_STONE_SKIN, 0},
    {SKILL_HIDE, SKILL_SNEAK, 0},
    {SKILL_VAMPSHIELD, 0},
    {0}
  };

  switch (type)
    {
    case SPEC_INIT:
      init_atts(alkian, 500, 500);
      init_skills(alkian, CLERIC_LEVEL_IND);
      init_skills(alkian, MAGE_LEVEL_IND);
      init_skills(alkian, DRUID_LEVEL_IND);
      init_skills(alkian, RANGER_LEVEL_IND);
      init_skills(alkian, THIEF_LEVEL_IND);
      init_skills(alkian, PSI_LEVEL_IND);
      return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(alkian, fight, peace, cmd));
    }

  return (FALSE);
}

SPECIAL(alkian_master)
{
  struct char_data *master = (struct char_data *) me;
  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK, SKILL_STUN, 0},
    {SPELL_FIREBALL, SPELL_DISINTEGRATE, 0},
    {SPELL_LAVA_STORM, SPELL_CALL_LIGHTNING, SPELL_POISON_GAS, 0},
    {SPELL_ACID_RAIN, SKILL_KICK, SPELL_CREEPING_DOOM, 0},
    {SKILL_PSIONIC_BLAST, SKILL_THOUGHT_THROW, 0},
    {SKILL_DODGE, SKILL_TRIP, SPELL_HEAL, 0},
    {0}
  };

  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_FIRESHIELD, 0},
    {SPELL_HEAL, SPELL_ARMOR, SPELL_SHIELD, 0},
    {0}
  };

  switch (type)
    {
    case SPEC_INIT:
      init_atts(master, 500, 500);
      init_skills(master, CLERIC_LEVEL_IND);
      init_skills(master, MAGE_LEVEL_IND);
      init_skills(master, DRUID_LEVEL_IND);
      init_skills(master, RANGER_LEVEL_IND);
      init_skills(master, THIEF_LEVEL_IND);
      init_skills(master, PSI_LEVEL_IND);
      return (TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(master, fight, peace, cmd));
    }

  return (FALSE);
}

SPECIAL(deep_warrior)
{
  struct char_data *warrior = (struct char_data *) me;

  static int fight[][MAX_SET_SIZE] = {
    {SPELL_DISPEL_MAGIC, SKILL_BERSERK,SPELL_SLEEP,  0},
    {SPELL_VAMPYRIC_TOUCH, SPELL_DISINTEGRATE, 0},
    {SKILL_BASH, SKILL_THROW, SKILL_DODGE, 0},
    {SKILL_STUN, SPELL_LAVA_STORM, SPELL_ACID_RAIN, 0},
    {SKILL_PSIONIC_BLAST, SKILL_THOUGHT_THROW, 0},
    {SPELL_WEAKNESS,SPELL_HEAL,SKILL_SCRY, 0},
    {0}
  };
  static int peace[][MAX_SET_SIZE] = {
    {SPELL_SANCTUARY, SPELL_HEAL, SPELL_FIRESHIELD, 0},
    {SPELL_SHIELD, SPELL_ARMOR, SPELL_STONE_SKIN, 0},
    {SPELL_ELECSHIELD, SPELL_COLDSHIELD, SPELL_POISONSHIELD, 0},
    {SKILL_VAMPSHIELD, SKILL_MANASHIELD, SPELL_MOVESHIELD, 0},
    {SPELL_BLESS,SKILL_SCRY,SKILL_THROW, 0},
    {0}
  };

  switch(type)
    {
    case SPEC_INIT:
      init_atts(warrior, 500, 500);
      init_skills(warrior, MAGE_LEVEL_IND);
      init_skills(warrior, PSI_LEVEL_IND);
      init_skills(warrior, CLERIC_LEVEL_IND);
      init_skills(warrior, RANGER_LEVEL_IND);
      init_skills(warrior, WARRIOR_LEVEL_IND);
      init_skills(warrior, DRUID_LEVEL_IND);
      return(TRUE);

    case SPEC_IDLE:
    case SPEC_FIGHT:
      return (do_abilities(warrior, fight, peace, cmd));
    }

  return(FALSE);
}

SPECIAL(N_Sonic_Wall)
{
    if (type == SPEC_INIT)
	return (FALSE);

    if (cmd != 1)
	return (FALSE);

    act("A solid wall of sound prevents your leaving in that direction!",
	TRUE, ch, 0, 0, TO_CHAR);
    return (TRUE);
}


void guard_help_call(struct char_data *guard, int level, int town)
{
    if (!check_soundproof(guard))
    {
	if (number(0,20) == 0)
	    do_comm(guard, "To me, my fellows! I am in need of thy aid!",
		    CMD_HOLLER);
	else
	    act("$n shouts 'To me, my fellows! I need thy aid!'",
		TRUE, guard, 0, 0, TO_ROOM);

	if (guard->specials.fighting)
	    CallForGuard(guard, guard->specials.fighting, level, town);
    }
}

int guard_idle_action(struct char_data *guard)
{
    struct char_data *tch;
    struct char_data *evil = NULL;
    int max_evil = 1000;

    if (check_peaceful(guard, ""))
	return FALSE;

    for (tch = real_roomp(guard->in_room)->people; tch;
	 tch = tch->next_in_room)
    {
	if(IS_SET(tch->specials.flags, PLR_PKILLER))
	{
	    if(!check_soundproof(guard))
		act("$n screams 'MURDERER!!!'", FALSE, guard, 0, 0, TO_ROOM);
	    hit(guard, tch, TYPE_UNDEFINED);
	    return TRUE;
	}
	if (tch->specials.fighting)
	{
	    if ((GET_ALIGNMENT(tch) < max_evil) &&
		(IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
	    {
		max_evil = GET_ALIGNMENT(tch);
		evil = tch;
	    }
	}
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0))
    {
	if (!check_soundproof(guard))
	    act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
		FALSE, guard, 0, 0, TO_ROOM);
	hit(guard, evil, TYPE_UNDEFINED);
	return(TRUE);
    }

    return (FALSE);
}
/*
SPECIAL(vampire_ss) {
     char objname[MAX_INPUT_LENGTH], victname[MAX_INPUT_LENGTH];
     int tcount=0;
     struct obj_data *obj, *token[10];
     struct char_data *mob = (char_data *) me;



     // list command
     if(cmd==59) {
        send_to_char("I can enhance weapons with the ability to drain the life\n\r",ch);
        send_to_char("of your enemies... but the price is high. I require 7 quest\n\r",ch);
        send_to_char("tokens, 100 mil expierience points, and 20 mil gold... give me\n\r", ch);
        send_to_char("the tokens, and then the item, I will take the rest...\n\r", ch);
        return TRUE;
     }

     if(cmd!=72) return FALSE;

     arg=one_argument(arg,objname);
     arg=one_argument(arg,victname);

     // makes sure they're giving the item to the vampire master
     if(!(isname(victname, "vampire-master")))
         return FALSE;

     // checks to make sure they have enough gold
     if(GET_GOLD(ch)<20000000) {
        send_to_char("Do you take me for a fool?\n\r", ch);
        send_to_char("You have not enough money to afford my services!!!\n\r", ch);
        return TRUE;
     }

     // checks to make sure they have enough exp
     if(GET_EXP(ch) < 100000000) {
        send_to_char("I cannot help you, you have no knowledge for me to feed upon\n\r", ch);
        return TRUE;
     }

     // token counter code...by me (quilan)
     for(obj = ch->carrying; obj ; obj=obj->next_content) {
        //A token's virt is 22006
        if(obj_index[obj->item_number].virt == 22006) {
           token[tcount++] = obj;
           if(tcount>=7) break;
        }
     }

     // checks to see if there are enough tokens
     if(tcount < 7) {
        send_to_char("I'm afraid you don't have enough tokens!\n\r", ch);
        return TRUE;
     }

     // checks to make sure they have the item
     if(!(obj=get_obj_in_list_vis(ch, objname, ch->carrying))) {
        send_to_char("I can not enhance what you do not have! Begone fool!\n\r", ch);
        return TRUE;
     }

     int a;

     // locates the open slot
     for(a=0;a<MAX_OBJ_AFFECT;a++)
        if(obj->affected[a].location==0)
            break;

     // quits if there is no open spot
     if(obj->affected[a].location!=0) {
        send_to_char("This item can recieve no more power...\n\r", ch);
        return TRUE;
     }

     obj->affected[a].location = 30;
     obj->affected[a].modifier = 74;

     // Output Text to players
     act("$N places his hand on the $p as a black aura sourounds it....", TRUE, ch, obj, mob, TO_CHAR);
     act("$N grins as he hands the $p back to you.", TRUE, ch, obj, mob, TO_CHAR);
     act("$N places his hand on the $p as a black aura sourounds it....", TRUE, ch, obj, mob, TO_ROOM);
     act("$N grins as he hands the $p back to $n.", TRUE, ch, obj, mob, TO_ROOM);

     // removes the tokens
     for(a=0;a<tcount;a++) {
       obj_from_char(token[a]);
       extract_obj(token[a]);
     }

     return TRUE;
}
*/      /* modified vamp_ss special for weapon forger, thanks quilian. gatz.*/
SPECIAL(vampire_ss) {
     char objname[MAX_INPUT_LENGTH], victname[MAX_INPUT_LENGTH];
     int tcount=0;
     struct obj_data *obj, *token[10];
     struct char_data *mob = (char_data *) me;

     // list command
     if(cmd==59) {
        send_to_char("I can enhance weapons with the ability to take the life\n\r",ch);
        send_to_char("of your enemies... but the price is high. I require 15 quest\n\r",ch);
      //  send_to_char("tokens, 100 mil expierience points, and 20 mil gold... give me\n\r", ch);
        send_to_char("tokens, give me the item, I will take the rest...Be warned,\n\r", ch);
        send_to_char("enhancing a weapon more then once is risky, do so at your own risk.\n\r", ch);
        send_to_char("If your weapon has 5 or more magical affects, you won't be refunded for greed.\n\r", ch);
        return TRUE;

     }

     if(cmd!=72) return FALSE;

     arg=one_argument(arg,objname);
     arg=one_argument(arg,victname);

     // makes sure they're giving the item to the vampire master
     if(!(isname(victname, "weapon-forger")))
     return FALSE;

     for(obj = ch->carrying; obj ; obj=obj->next_content) {
        //A token's virt is 22006
        if(obj_index[obj->item_number].virt == 22006) {
           token[tcount++] = obj;
           if(tcount>=15) break;
        }
     }

     // checks to see if there are enough tokens
     if(tcount < 15) {
        send_to_char("I'm afraid you don't have enough tokens!\n\r", ch);
        return TRUE;
     }

     // checks to make sure they have the item
     if(!(obj=get_obj_in_list_vis(ch, objname, ch->carrying))) {
        send_to_char("I can not enhance what you do not have! Begone fool!\n\r", ch);
        return TRUE;
     }

     // checks to make sure it is a weapon
     if(!(IS_WEAPON(obj))) {
       send_to_char("I am only an expert in weapons. Off with you!\n\r",ch);
       return TRUE;
     }

     int a;

     // locates the open slot
     for(a=0;a<MAX_OBJ_AFFECT;a++)
        if(obj->affected[a].location==0)
            break;

     // quits if there is no open spot
     if(obj->affected[a].location!=0) {
        send_to_char("This item can recieve no more power...\n\r", ch);
        return TRUE;
     }

     obj->affected[a].location = 40;
     obj->affected[a].modifier = 8;

     // Output Text to players
     act("$N begins to work on the $p pounding it with a hammer....", TRUE, ch, obj, mob, TO_CHAR);
     act("$N grins as he hands the $p back to you.", TRUE, ch, obj, mob, TO_CHAR);
     act("$N begins to work on the $p pounding it with a hammer....", TRUE, ch, obj, mob, TO_ROOM);
     act("$N grins as he hands the $p back to $n.", TRUE, ch, obj, mob, TO_ROOM);

     // removes the tokens
     for(a=0;a<tcount;a++) {
       obj_from_char(token[a]);
       extract_obj(token[a]);
     }

     return TRUE;
}

// For the avatar of Mallune in the Celestiel interlude
// Triggers on getting an item from player
// Will reward player for bringing her 3 items, and will
// remove all 3 items from char replacing it with something new.
// - raist.
SPECIAL(avatar_mallune) {
  char objname[MAX_INPUT_LENGTH];
  char victname[MAX_INPUT_LENGTH];
  const int NUMBER_OF_OBJECTS = 3;
  struct obj_data *obj, *objects[NUMBER_OF_OBJECTS];
  struct char_data *mob = (char_data *) me;
  struct obj_data *prize;
  char buf[255];

  // give command
  if(cmd!=72) return FALSE;
  
  arg=one_argument(arg,objname);
  arg=one_argument(arg,victname);
  
  // checks to make sure they have the item
  if(!(obj=get_obj_in_list_vis(ch, objname, ch->carrying))) {
    send_to_char("You do not have that item.", ch);
    return TRUE;
  }
  
  // 18504 = vnum for beautiful charming comb
  if (!(obj_index[obj->item_number].virt == 18504)) {
    act("$N says 'I don't want that, you keep it.", TRUE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }
  
  // Check that all 3 items are present and store in an array of objects
  int i = 0;
  for(obj = ch->carrying; obj && i < NUMBER_OF_OBJECTS; obj=obj->next_content, i++) {
    
    if(obj_index[obj->item_number].virt == 18504 || // Beautiful charming comb
       obj_index[obj->item_number].virt == 18507 || // Mirror of regeneration
       obj_index[obj->item_number].virt == 18522) { // Fumin Gene Gnome Project Book
      
      // ensure the same object is not counted twice
      if (obj != objects[0] && obj != objects[1]) {
	objects[i] = obj;
      }
    }
  }
  
  // remove objects from character
  for( i = 0; i < NUMBER_OF_OBJECTS; i++ ) {
    // Don't notify player of the comb, since he actually gave it to her (just take it anyway)
    if (!(obj_index[objects[i]->item_number].virt == 18504)) {
      sprintf(buf,"%s is removed swiftly from your inventory by an unseen force\n",OBJ_SHORT(objects[i]));
      send_to_char(buf, ch);
    }

    obj_from_char(objects[i]);
  }
  
  // Give new object to player
  prize = make_object(18510, VIRTUAL);
  act("$N gives you $p as a thank you for your services", TRUE, ch, prize, mob, TO_CHAR);
  obj_to_char(prize, ch);
  
  return TRUE;
}

