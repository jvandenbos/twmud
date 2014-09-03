
#include "config.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "opinion.h"
#include "race.h"
#include "multiclass.h"
#include "casino.h"
#include "fight.h"
#include "utility.h"
#include "act.h"
#include "spell_util.h"
#include "constants.h"
#include "spec.h"
#include "spell_procs.h"
#include "find.h"
#include "util_str.h"
#include "statistic.h"
#include "proto.h"
#include "stdlib.h"
#include "varfunc.h"

Variable *get_mob_var(struct char_data *ch, char *name);
   
int can_pkill(struct char_data *ch, struct char_data *victim) {
    struct char_data *c1=ch, *c2=victim;
    Variable *v1, *v2;

    if(IS_GOD(c1)) return 1;

    if(IS_POLY_PC(c1)) c1=real_character(c1);
    if(IS_POLY_PC(c2)) c2=real_character(c2);

    if(!IS_PC(c2)) return 1;

    v1 = get_mob_var(c1, "ctf_team");
    v2 = get_mob_var(c2, "ctf_team");
    if(v1 && v2) {
       if((v1->Value() > 0) && (v2->Value() > 0) &&
          (v1->Value() == v2->Value()) &&
          (PKILLABLE == 2))
         return 0;
    }
   
    // if the npc is charmed check for pkill on master 
    if(IS_NPC(c1) && c1->master) {
       if(IS_PC(c1->master)) {
	if (!IS_SET(RM_FLAGS(ch->in_room), ARENA))
	  if( (!c2->player.guildinfo.inguild() && (PKILLABLE!=2)) ||
	      (!c1->master->player.guildinfo.inguild() && (PKILLABLE!=2)) ||
	      ((c1->master->player.guildinfo.inguild() == 
		c2->player.guildinfo.inguild()) && (PKILLABLE!=2)) ||
	      ((abs(GetMaxLevel(c1->master)-GetMaxLevel(c2))>10 && PKILLABLE!=2) ||
	       (abs(GetMaxLevel(c1->master)-GetMaxLevel(c2))>100 && PKILLABLE==2)) ||
	      (GetMaxLevel(c1->master)<5) || (GetMaxLevel(c2)<5) ||
	      (!IS_SET(c1->master->player.pkillinfo.flags, CAN_PKILL)&&(PKILLABLE!=2)) ||
	      (!IS_SET(c2->player.pkillinfo.flags, CAN_PKILL)&&(PKILLABLE!=2)) ||
	      (IS_SET(c1->master->specials.flags, PLR_PKILLER)) ||
	      (!PKILLABLE) )
	    return 0;
	return 1;
      } else {
	return 1;
      }
    } else if (IS_NPC(c1))
      return 1;

    if (!IS_SET(RM_FLAGS(ch->in_room), ARENA))
//	if(!IS_SET(c2->specials.flags, PLR_PKILLER))
	    if( (!c2->player.guildinfo.inguild() && (PKILLABLE!=2)) ||
                (!c1->player.guildinfo.inguild() && (PKILLABLE!=2)) ||
	        ((c1->player.guildinfo.inguild() == 
	         c2->player.guildinfo.inguild()) && (PKILLABLE!=2)) ||
	        ((abs(GetMaxLevel(c1)-GetMaxLevel(c2))>10 && PKILLABLE!=2) ||
		 (abs(GetMaxLevel(c1)-GetMaxLevel(c2))>100 && PKILLABLE==2)) ||
		(GetMaxLevel(c1)<5) || (GetMaxLevel(c2)<5) ||
		(!IS_SET(c1->player.pkillinfo.flags, CAN_PKILL)&&(PKILLABLE!=2)) ||
		(!IS_SET(c2->player.pkillinfo.flags, CAN_PKILL)&&(PKILLABLE!=2)) ||
		(IS_SET(c1->specials.flags, PLR_PKILLER)) ||
		(!PKILLABLE) )
		 return 0;
    return 1;
}

void do_hit(struct char_data *ch, char *argument, int cmd) 
{
    
    char arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    struct follow_type *k;
    ACMD(do_assist);
    struct char_data *targ = NULL;
    int i, found=FALSE;
    
    if (check_blackjack(ch)) {
	do_bj_hit(ch, argument, cmd);
	return;
    }
    
    if(GET_POS(ch) < POSITION_FIGHTING) {
	send_to_char("Maybe you should get on your feet first.",ch);
	return;
    }
    
    if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
	return;
    
    only_argument(argument, arg);
    
    if (*arg)
    {
	victim = get_char_room_vis(ch,arg);
	
	if (victim)
	{
	    if (victim == ch)
	    {
		send_to_char("You hit yourself..OUCH!.\n\r", ch);
		act("$n hits $mself, and says OUCH!",
		    FALSE, ch, 0, victim, TO_ROOM);
	    }
	    else
	    {
		if (!IS_NPC(victim) && (victim->invis_level > TRUST(ch)))
		{
		    send_to_char("They aren't here.\n\r", ch);
		    return;
		}
		
		if (IS_PC(ch) && IS_PC(victim) || PKILLABLE==2) {  /* if combat between PC's */
		    if(!can_pkill(ch, victim)) {
			send_to_char("I'm sorry...you can not hit that player.\n\r", ch);
			return;
		    }
		}

		if (IS_GOD(victim)) /* no no to attacking gods */
		{
		    send_to_char("Y'know, I really dont think thats a good idea!\n\r", ch);
		    return;
		}
		
		if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
		    act("$N is just such a good friend, you simply can't hit $M.",
			FALSE, ch,0,victim,TO_CHAR);
		    return;
		}
		
		if (((GET_POS(ch)==POSITION_STANDING) ||
		     (GET_POS(ch)==POSITION_MOUNTED)) &&
		    (victim != ch->specials.fighting)) {
		    hit(ch, victim, TYPE_UNDEFINED);
		    WAIT_STATE(ch, PULSE_VIOLENCE+2);

		} else {
		    send_to_char("You do the best you can!\n\r",ch);
		}
	    }
	} else {
	    send_to_char("They aren't here.\n\r",ch);
	}
    } else {
	send_to_char("Hit who?\n\r", ch);
    }
}



void do_kill(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    
    if ((TRUST(ch) < TRUST_LRGOD) || IS_NPC(ch))
    {
	do_hit(ch, argument, 0);
	return;
    }
    
    only_argument(argument, arg);
    
    if (!*arg)
	send_to_char("Kill who?\n\r", ch);
    
    else
    {
	if (!(victim = get_char_room_vis(ch, arg)) ||
	    (!IS_NPC(victim) && (!CAN_SEE(ch,victim))))
	    send_to_char("They aren't here.\n\r", ch);
	else if (ch == victim)
	    send_to_char("Your mother would be so sad.. :(\n\r", ch);
	else if (((TRUST(ch) < TRUST_LORD) && (IS_NPC(victim))) ||
		 (TRUST(ch) >= TRUST_LORD)) {
	    act("You chop $M to pieces! Ah! The blood!",
		FALSE, ch, 0, victim, TO_CHAR);
	    act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
	    act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
	    raw_kill(victim);
	}
    }
}

void do_order(struct char_data *ch, char *argument, int cmd)
{
   char name[MAX_INPUT_LENGTH], message[256];
   char buf[256];
   bool found = FALSE;
   room_num org_room;
   struct char_data *victim;
   struct follow_type *k;
   
   
   if (apply_soundproof(ch))
     return;
   
   half_chop(argument, name, message);
   
   if (!*name || !*message)
     send_to_char("Order who to do what?\n\r", ch);
   else if (!(victim = get_char_room_vis(ch, name)) &&
	    str_cmp("follower", name) && str_cmp("followers", name))
     send_to_char("That person isn't here.\n\r", ch);
   else if (ch == victim)
     {
	send_to_char(
		     "You obviously suffer from Multiple Personality Disorder.\n\r", ch);
     }
   else if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char(
		   "Your superior would not aprove of you giving orders.\n\r",ch);
      return;
   } else if(victim) {
      if (check_soundproof(victim))
      	return;
      
      if(!IS_NPC(ch) && !IS_NPC(victim)) {
	 send_to_char("You can't just go ordering other people around!\n\r", ch);
	 return;
      }
      
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
      
      if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
	{
#if 0
	   if (RIDDEN(victim) == ch) {
	      int check;
	      check = MountEgoCheck(ch, victim);
	      if (check > 5) {
		 if (RideCheck(ch, -5)) {
		    act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
		 } else {
		    Dismount(ch, victim, POSITION_SITTING);
		    act("$n gets pissed and $N falls on $S butt!", 
			FALSE, victim, 0, ch, TO_NOTVICT);
		    act("$n gets pissed you fall off!", 
			FALSE, victim, 0, ch, TO_VICT);
		 }
	      } else if ((check > 0) || IsImmune(victim, IMM_CHARM)){
		 act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	      } else {
		 send_to_char("Ok.\n\r", ch);
		 command_interpreter(victim, message, 1);
	      }
	   } else {
	      act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	   }
#else
	   act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
#endif
	}
      else {
	 send_to_char("Ok.\n\r", ch);
	 command_interpreter(victim, message, 1);
      }
   } else {/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);
      
      org_room = ch->in_room;
      
      for (k = ch->followers; k; k = k->next) {
	 if (org_room == k->follower->in_room)
	   if (IS_AFFECTED(k->follower, AFF_CHARM) && IS_NPC(k->follower)) {
	      found = TRUE;
	      command_interpreter(k->follower, message, 1);
	   }
      }
      if (found)
      	send_to_char("Ok.\n\r", ch);
      else
      	send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
   }
}

void do_flee(struct char_data *ch, char *argument, int cmd) {
   int i, attempt, loose, die, percent, was_room;

   if(IS_AFFECTED(ch, AFF_BERSERK)) {
       if (cmd)
	 send_to_char_formatted("$CYFlee ???$CR NEVER !$CN\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_PARALYSIS))
     return;
   
   if (ch->specials.binded_by) {
      send_to_char_formatted("$CYYou struggle to flee but can't!$CN\n\r", ch);
      return;
   }
   
   if (affected_by_spell(ch, SPELL_WEB)) {
      if (!saves_spell(ch, SAVING_PARA, 0)) {
	 WAIT_STATE(ch, PULSE_VIOLENCE*10);
	 send_to_char_formatted("$CrYou are ensared in webs, "
				"you cannot move!$CN\n\r", ch);
	 act("$Cg$n struggles against the webs that hold $m$CN", TRUE, ch, 0, 0,
	     TO_ROOM);
	 return;
      } else {
	send_to_char_formatted("$CGYou pull free from the sticky webbing!$CN\n\r", 
			       ch);
	act("$CG$n manages to pull free from the sticky webbing!$CN", TRUE, ch, 0,
	    0, TO_ROOM);
      }
   }
   
   if (GET_POS(ch)<POSITION_FIGHTING) {
      act("$CG$n scrambles madly to $s feet!$CN",TRUE,ch,0,0,TO_ROOM);
      act("$CGPanic-stricken, you scramble to your feet!$CN",TRUE,ch,0,0,TO_CHAR);
      GET_POS(ch) = POSITION_STANDING;
      WAIT_STATE(ch, PULSE_VIOLENCE*4);
      return;
   }
   
   /* If character isn't fighting AND this was a character action
    * i.e. not called from somewhere else in which case cmd==0,
    * then dont' flee
    */
   if (!ch->specials.fighting && cmd) {
     act("$CY$n looks around nervously.$CN",TRUE,ch,0,0,TO_ROOM);
     act("$CYYou coward!  From whom are you fleeing?$CN",TRUE,ch,0,0,TO_CHAR);
     return;
   }
   

   /* auto 30% chance of failing flee - I didn't see an actual chance in here so.. */

   if (number(1,9) < 3)
     {
       send_to_char_formatted("$CRPANIC!$Cw You couldn't escape!$CN\n\r", ch);
       return;
     }


   for(i = ch->specials.fighting ? 3 : 6 ; i > 0 ; --i) {
     attempt = number(0, 5);	/* Select a random direction */
     if (CAN_GO(ch, attempt) &&
	 !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
       act("$CY$n panics, and attempts to flee.$CN", TRUE, ch, 0, 0, TO_ROOM);
       was_room = ch->in_room;
       if (RIDDEN(ch)) {
	 die = MoveOne(RIDDEN(ch), attempt);
       } else {
	 die = MoveOne(ch, attempt);
       }
       if (die == 1)
	 { 
	   /* The escape has succeded. We'll be nice. */
	   if(ch->specials.fighting)
	     {
	       if(GetMaxLevel(ch) > 3) {
		 loose = GetMaxLevel(ch)+(GetSecMaxLev(ch)/2) +
		   (GetThirdMaxLev(ch)/3);
		 loose -= GetMaxLevel(ch->specials.fighting) +
		   (GetSecMaxLev(ch->specials.fighting)/2) +
		   (GetThirdMaxLev(ch->specials.fighting)/3);
		 loose *= GetMaxLevel(ch);
	       } else {
		 loose = 0;
	       }
	       if (loose < 0) 
		 loose = 0;

	       if(!IS_PC(ch))
		 {
		   if (!IS_SET(ch->specials.mob_act, ACT_AGGRESSIVE))
		     {
		       AddFeared(ch, ch->specials.fighting);
		     }
		   else /* if its an AGG or META_AGG mob -- HUNT! */
		     {
		       //percent=(int)(100. * (double) GET_HIT(ch->specials.fighting) /
		       //		     (double) GET_MAX_HIT(ch->specials.fighting));
		       //if (number(1,101) < percent) {
		       //	 if ((Hates(ch->specials.fighting, ch)) ||
			//     (IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
			//     (IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))) {
			   SetHunting(ch->specials.fighting, ch);
		//	 }
		      // }
		     }
		 }
	       else if(loose > 0)
		 gain_exp(ch, -loose);
	     }
	   
	   send_to_char_formatted("$CYYou flee head over heels.$CN\n\r", ch);
	   stop_opponents(ch, was_room);
	   stop_fighting(ch);
	 }
       else
	 {
	   act("$CR$n tries to flee, but is too exhausted!$CN",
	       TRUE, ch, 0, 0, TO_ROOM);
	 }
       return;
     }
   } /* for */
   
   /* No exits were found */
   send_to_char_formatted("$CRPANIC!$Cw You couldn't escape!$CN\n\r", ch);
}

void do_assist(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim, *tmp_ch;
  char victim_name[240];
  
  if (check_peaceful(ch,"No one should need assistance here.\n\r"))
    return;
  
  only_argument(argument, victim_name);
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to assist?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Oh, by all means, help yourself...\n\r", ch);
    return;
  }
  
  if (ch->specials.fighting == victim) {
    send_to_char("That would be counterproductive?\n\r",ch);
    return;
  }
  
  if (ch->specials.fighting) {
    send_to_char("You have your hands full right now\n\r",ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to assist!\n\r", ch);
    return;
  }

  
  tmp_ch = victim->specials.fighting;
  /*	for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
	(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
	*/
  if (!tmp_ch) {
    act("But $E's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
   
  if(!can_pkill(ch, tmp_ch)) {
     cprintf(ch, "You're not allowed to kill that person.\n\r");
     return;
  }
  
  hit(ch, tmp_ch, TYPE_UNDEFINED);
  
  WAIT_STATE(victim, PULSE_VIOLENCE+2); /* same as hit */
}

void do_wimp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int wimpy=-1;
  
  /* sets the character in wimpy mode.  */

 if(*argument)
   scan_number(argument, &wimpy);
  
  if (wimpy > 32767) wimpy = 32767;
    
  /* If no wimpy level specified, set/unset default wimpy settings */
  if(wimpy < 0)
  {
    if (IS_SET(ch->specials.flags, PLR_WIMPY)) {
      REMOVE_BIT(ch->specials.flags, PLR_WIMPY);
      send_to_char("Ok, you are no longer a wimp...\n\r",ch);
      GET_WIMPY(ch)=0;
    } else {
      SET_BIT(ch->specials.flags, PLR_WIMPY);
      GET_WIMPY(ch)=0;  /* Default wimpy */
    }
  }
  else /* a specific wimpy setting was specified, so set it */
  {
    SET_BIT(ch->specials.flags, PLR_WIMPY);
    GET_WIMPY(ch)=wimpy;
  }
  /* If ch is set wimpy, display status of wimpy setting, level or default */
  if (IS_SET(ch->specials.flags, PLR_WIMPY))
  {
    if(GET_WIMPY(ch))
      sprintf(buf,"Ok, your wimpy level is set to %i hit points.\n\r",
	     GET_WIMPY(ch));
    else
      sprintf(buf,"Ok, your now in default wimpy mode.\n\r");
    send_to_char(buf, ch);
  }
}

/* 5 breath types ranked in ascending order of strength */
static struct breaths bweapons[] = {    /* func, mana cost */
  { cast_poison_gas_breath, 10 },
  { cast_frost_breath, 20 }, 
  { cast_lightning_breath, 30 }, 
  { cast_acid_breath, 40 }, 
  { cast_fire_breath, 50 }
};

void do_breath(struct char_data *ch, char *argument, int cmd)
{
  int i, index, mana_cost = 0;
  breath_func func = NULL;
  char breath_type[255];

  if (argument) {
    one_argument(argument, breath_type);
  }

  if (check_peaceful(ch,"That wouldn't be nice at all.\n\r"))
    return;

  if (GET_RACE(ch) != RACE_DRAGON &&
       !IS_AFFECTED2(ch, AFF2_FIRE_BREATH) &&
       !IS_AFFECTED2(ch, AFF2_FROST_BREATH) &&
       !IS_AFFECTED2(ch, AFF2_ACID_BREATH) &&
       !IS_AFFECTED2(ch, AFF2_POISONGAS_BREATH) &&
       !IS_AFFECTED2(ch, AFF2_LIGHTNING_BREATH)
      )
      {
	send_to_char("You aren't a dragon. Try not brushing for a while if you want killer breath.\n\r", ch);
	return;
      }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("You are too indecisive to do it.\n\r",ch);
    return;
  }

  index = -1;
  if (GET_RACE(ch) == RACE_DRAGON) {
    /* choose best of the 5 breath weapons based on level */
    index = MAX(MIN(GetMaxLevel(ch)/10, 4), 0);  /* 0 - 4 */
  } else {

    if (!strcmp(breath_type, "poisongas")) {
      index = 0;
      if (!IS_AFFECTED2(ch, AFF2_POISONGAS_BREATH)) {
	send_to_char("You cannot use poisongas breathweapons you do not understand\r\n", ch);
	return;
      }
    }

    if (!strcmp(breath_type, "frost"))  {
      index = 1;
      if (!IS_AFFECTED2(ch, AFF2_FROST_BREATH)) {
	send_to_char("You cannot use frost breathweapons you do not understand\r\n", ch);
	return;
      }
    }

    if (!strcmp(breath_type, "lightning")) {
      index = 2;
      if (!IS_AFFECTED2(ch, AFF2_LIGHTNING_BREATH)) {
	send_to_char("You cannot use lightning breathweapons you do not understand\r\n", ch);
	return;
      }
    }

    if (!strcmp(breath_type, "acid")) {
      index = 3;
      if (!IS_AFFECTED2(ch, AFF2_ACID_BREATH)) {
	send_to_char("You cannot use acid breathweapons you do not understand\r\n", ch);
	return;
      }
    }
    
    if (!strcmp(breath_type, "fire")) {
      index = 4;
      if (!IS_AFFECTED2(ch, AFF2_FIRE_BREATH)) {
	send_to_char("You cannot use fire breathweapons you do not understand\r\n", ch);
	return;
      }
    }
  }

  if (index < 0) {
    send_to_char("You must specify either: poisongas, frost, lightning, acid, fire\r\n",ch);
    return;
  }

  /* if we don't have the mana for it, then try next lower one */
  for (i=index; i>=0; i--) {
    if (GET_MANA(ch) >= bweapons[i].mana_cost) {
      func = bweapons[i].func;
      mana_cost = bweapons[i].mana_cost;
      break;
    }
  }

  breath_weapon(ch, mana_cost, func);
  
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_shoot(struct char_data *ch, char *argument, int cmd)
{
  char arg[80];
  struct char_data *victim;
  
  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
    return;

  only_argument(argument, arg);
  
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
	send_to_char("You can't shoot things at yourself!", ch);
	return;
      } else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't shoot at $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	if (ch->specials.fighting) {
	  send_to_char("You're at too close range to fire a weapon!\n\r", ch);
	  return;
	}
	shoot(ch, victim);
	WAIT_STATE(ch, PULSE_VIOLENCE);
      }
    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Shoot who?\n\r", ch);
  }
}
