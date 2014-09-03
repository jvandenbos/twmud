
/* ************************************************************************
 *  file: mobact.c , mobile action module.                 part of dikumud *
 *  usage: procedures generating 'intelligent' behavior in the mobiles.    *
 *  copyright (c) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "opinion.h"
#include "trap.h"
#include "spells.h"
#include "state.h"
#include "act.h"
#include "multiclass.h"
#include "fight.h"
#include "utility.h"
#include "constants.h"
#include "mobact.h"
#include "hash.h"
#include "skills.h"
#include "util_str.h"
#include "track.h"
#include "sound.h"
#include "spec.h"
#include "mobprog2.h"

#include "proto.h"

/* forward declarations */
int AssistFriend( struct char_data *ch);
int SameRace( struct char_data *ch1, struct char_data *ch2);
int MobFriend( struct char_data *ch, struct char_data *f);
int GetDamage(struct obj_data *w, struct char_data *ch);
int GetDamBonus(struct obj_data *w);
int GetHandDamage (struct char_data *ch);
int mobile_sounds(struct char_data* ch);

struct char_data *aggro_scan(struct char_data *mob, int range, int cnd, int *tdr) 
{
   int i, r, rm = mob->in_room;
   struct char_data *spud;

   for (i=0;i<6;i++) {
      r = 0;
      rm = mob->in_room;
      while (r<range) {
         if (clearpath(mob, rm,i)) {
            rm = real_roomp(rm)->dir_option[i]->to_room;
            r++;
            for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
               if ((CAN_SEE(mob,spud))&&(!IS_MOB(spud))&&!IS_IMMORTAL(spud)) {
                  if(!IS_SET(mob->specials.mob_act,ACT_STAY_ZONE) ||
		     (real_roomp(rm)->zone==real_roomp(mob->in_room)->zone))
		  {
                     *tdr = i; 
                     if (cnd==0) return spud;
/*
                     if ((cnd==1)&&(is_pissed(mob,spud))) return spud;
*/
                  }
               }
            }
         } else {
            r = range + 1;
         }
      }
   }
   return NULL;
}

int mobile_guardian(struct char_data *ch)
{
    struct char_data *targ = NULL;
    int i, found=FALSE;
  
    if (ch->in_room > NOWHERE) {
	if ((!ch->master) || (!IS_AFFECTED(ch, AFF_CHARM)))
	    return FALSE;
	if (ch->master->specials.fighting) { /**/
	    for(i=0;i<10&&!found;i++) {
		targ = FindAnAttacker(ch->master);
		if (targ)
		    found=TRUE;
	    }
      
	    if (!found) return FALSE;
      
	    if (!SameRace(targ, ch)) {
		if (IsHumanoid(ch)) {
		    act("$n screams 'I must protect my master!'", 
			FALSE, ch, 0, 0, TO_ROOM);
		} else {
		    act("$n growls angrily!", 
			FALSE, ch, 0, 0, TO_ROOM);
		}
		if ((CAN_SEE(ch, targ)) && (ch->in_room == ch->master->in_room))
		{
		    hit(ch, targ,0);
		    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

int mobile_wander(struct char_data *ch)
{
  int	door = -1;
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  
  if (! ((GET_POS(ch) == POSITION_STANDING) &&
	 ((door = number(0, 15)) <= 5) &&
	 exit_ok(exitp=EXIT(ch,door), &rp) &&
	 !IS_SET(rp->room_flags, NO_MOB) &&
	 !IS_SET(rp->room_flags, DEATH))
      )
    return FALSE;
  
  if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
    if (ch->specials.last_direction == door) {
      ch->specials.last_direction = -1;
    } else {
      if (!IS_SET(ch->specials.mob_act, ACT_STAY_ZONE) ||
	  (rp->zone == real_roomp(ch->in_room)->zone)) {
	ch->specials.last_direction = door;
	go_direction(ch, door, 0);
	return TRUE;
      }
    }
  }
  return FALSE;
}

void ClearHunt (struct char_data *ch)
{
    path_kill(ch->hunt_info);
    ch->hunt_info = 0;
}

void FoundHated (struct char_data *ch, struct char_data *targ)
{
  if (check_peaceful(ch, "This room is too peaceful to fight in.\n\r")) {
    return;
  }

  if (IsHumanoid(ch)) {
      act ("You see $N and fumes with rage...",TRUE,ch,0,targ,TO_CHAR);
      act ("$n sees $N and fumes with rage...",TRUE,ch,0,targ,TO_NOTVICT);
      act ("$n sees you and fumes with rage...",TRUE,ch,0,targ,TO_VICT);
  } else {
      act ("You growl ferociously at $N...",TRUE,ch,0,targ,TO_CHAR);
      act ("$n growls ferociously at $N...",TRUE,ch,0,targ,TO_NOTVICT);
      act ("$n growls ferociously at you...",TRUE,ch,0,targ,TO_VICT);
  }

  hit (ch, targ, 0);
}

int MobHunt(struct char_data *ch)
{
  int res;
  room_num dest;
  struct char_data* vict;
  
  if(!ch->hunt_info)
    return 0;
  
  if(ch->persist<=0)
  {
    path_kill(ch->hunt_info);
    ch->hunt_info = 0;
    return 0;
  }
  
  ch->persist -= 1;
  vict = ch->hunt_info->victim;
  dest = ch->hunt_info->dest;

  if((res = track(ch)) != -1)
  {
    go_direction(ch, res, 1);
    if(vict)
    {
      if(vict->in_room == ch->in_room)
      {
	if(Hates(ch, vict))
	  FoundHated (ch, vict);
	path_kill(ch->hunt_info);
	ch->hunt_info = 0;
      }
    }
    else if(dest == ch->in_room)
    {
      path_kill(ch->hunt_info);
      ch->hunt_info = 0;
    }
  }

  return 0;
}

int MobScavenge(struct char_data *ch)
{
  struct obj_data *best_obj=0, *obj=0;
  int max;
  
  if ((real_roomp(ch->in_room))->contents && !number(0,5)) {
    for (max = 1,best_obj = 0,obj = (real_roomp(ch->in_room))->contents;
	 obj; obj = obj->next_content) {
      // If mob can take object and make sure mobs don't pick up flowers
      if (CAN_GET_OBJ(ch, obj) && !(GET_OBJ_RNUM(obj) < 8751) && !(GET_OBJ_RNUM(obj)>8715) ) {
	if (obj->obj_flags.cost > max) {
	  best_obj = obj;
	  max = obj->obj_flags.cost;
	}
      }
    } /* for */
    
    if (best_obj) {
      if (CheckForAnyTrap(ch, best_obj))
	return TRUE;
      
      obj_from_room(best_obj);
      obj_to_char(best_obj, ch);
      act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
      if((!CAN_WEAR(best_obj, ITEM_WIELD)) && (can_wear_test(ch,best_obj))) 
	do_wear(ch,fname(OBJ_NAME(best_obj)),0); /*** wear item ***/
      return TRUE;
    }
  }
  return FALSE;
}

void check_mobile_activity(int pulse)
{
    register struct char_data *ch; 
    int tick;

    tick = (pulse / PULSE_MOBILE) % MOB_DIVISOR;
    
    EACH_CHARACTER(iter,ch)
      {
	if (IS_NPC(ch))
	  {
	    /* note:
	     *   This should be non_violent_mobile_activity,
	     *   cuz fighting activity is elsewhere
	     *   --mnemosync
	     */
	    if (abs(ch->specials.tick % MOB_DIVISOR) == tick)
	      mobile_activity(ch);
	    else if (ch->hunt_info)
	      MobHunt(ch);
	  }
      }
    END_AITER(iter);

#ifdef JANWORK  
    tick = (pulse / PULSE_MOBILE) % TICK_WRAP_COUNT;
  
    EACH_CHARACTER(iter, ch)
    {
      if(IS_NPC(ch))
      {
	if(ch->specials.tick == tick)
	  mobile_activity(ch);
	else if(ch->hunt_info)
	  MobHunt(ch);
      }
    }
    END_AITER(iter);
#endif
}

void mobile_activity(struct char_data *ch)
{
  /* The comments in this procedure is a mess.
   * I'm not hoping to figure out the history
   * but if ppl make comments, please remember
   * to (re)move them with the proper code.
   * --Mnemosync
   */

  struct char_data *tmp_ch;
  char buf[MAX_STRING_LENGTH];
  
  /* Examine call for special procedure */
  
  /* some status checking for errors */
  if(!real_roomp(ch->in_room))
    {
      log_msg("Char not in correct room.  moving to 50 ");
      char_from_room(ch);
      char_to_room(ch, 50);
    }

  /* polies don't do auto actions */
  if(IS_SET(ch->specials.mob_act, ACT_POLYSELF))
    return;
  
  if (IS_SET(ch->specials.mob_act, ACT_SPEC) && !no_specials && !ch->specials.fighting) {
    if (!mob_index[ch->nr].func) {
      sprintf(buf, "Attempting to call a non-existing MOB func. %s Vnum: %d", GET_IDENT(ch), 
		  mob_index[ch->nr].virt);
      log_msg(buf);
      REMOVE_BIT(ch->specials.mob_act, ACT_SPEC);
    } else {
      STATE1("Calling mob special function: %s", GET_NAME(ch));
      if ((*mob_index[ch->nr].func)(ch, 0, 0, "", SPEC_IDLE))
	return;
    }
  }
  
  /* check to see if the monster is possessed */

  STATE1("Doing mob actions: %s", GET_NAME(ch));
  
  /* if we're already fighting or sleeping don't do any of this stuff */
  if(AWAKE(ch) && !ch->specials.fighting)
    {
      if (ch->in_room <= NOWHERE) {
	sprintf(buf, "Before mprog_random_trigger: Mob: %s is not in not in a valid room, "
		"currently in: %ld", GET_IDENT(ch), ch->in_room);
	log_msg(buf);
      }
      mprog_random_trigger(ch);
      mprog_random_trigger2(ch);
      
      if (ch->in_room <= NOWHERE) {
	sprintf(buf, "After mprog_random_trigger: Mob: %s is not in not in a valid room, "
		"currently in: %ld", GET_IDENT(ch), ch->in_room);
	//	log_msg(buf);
	return;
      }
      
      if (ch->mpactnum > 0) {
	MPROG_ACT_LIST *tmp_act, *tmp2_act;
	for (tmp_act = ch->mpact; tmp_act != NULL; tmp_act = tmp2_act) {
	  mprog_wordlist_check(tmp_act->buf, ch, tmp_act->ch,
			       tmp_act->obj, tmp_act->vo, ACT_PROG);
	  tmp2_act = tmp_act->next;
	  FREE(tmp_act->buf);
	  FREE(tmp_act);
	}
	ch->mpactnum = 0;
	ch->mpact = NULL;
      }
      
      if(AssistFriend(ch))
	return;
      
      if (IS_SET(ch->specials.mob_act, ACT_SCAVENGER) && MobScavenge(ch))
	return;
      
      if(MobHunt(ch))
	return;
      
      if ((!IS_SET(ch->specials.mob_act, ACT_SENTINEL) && 
	   !IS_SET(ch->specials.mob_act, ACT_POLYSELF)) && mobile_wander(ch))
	return;
      
      if ( IS_SET(ch->specials.mob_act, ACT_AFRAID) &&
	   (GET_HIT(ch) > (GET_MAX_HIT(ch)/2)) &&
	   ((tmp_ch = FindAFearee(ch)) != NULL))
	{
	  do_flee(ch, "", 0);
	  return;
	}
      
      if (IS_SET(ch->specials.mob_act, ACT_HATEFUL) &&
	  (tmp_ch = FindAHatee(ch)))
	{
	  FoundHated (ch, tmp_ch);
	  return;
	}
      
      if (!ch->specials.fighting &&
	  IS_SET(ch->specials.mob_act, ACT_AFRAID) &&
	  (tmp_ch = FindAFearee(ch))!= NULL)
	{
	  do_flee(ch, "", 0);
	  return;
	}
      
      if (IS_SET(ch->specials.mob_act,ACT_AGGRESSIVE)) {
	tmp_ch = FindVictim(ch);
	if (tmp_ch) {
	  if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n\r")) {
	    act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	  
	  /* check to see if the player is aggressive towards aggressive mobs */
	  if (IS_SET(tmp_ch->specials.flags,PLR_AGGR)
	      && (!tmp_ch->specials.fighting) &&
	      CAN_SEE(tmp_ch, ch)) {
	    act("$n fumes with rage and attacks $N for no reason!",TRUE,tmp_ch,0,ch,TO_ROOM);
	    
	    act("You fume with rage and heroicly attack $N!",FALSE,tmp_ch,0,ch,TO_CHAR);
	    /* check to see if good mob attacks good paladin and makes
	       the mob neutral */
	    if (HasClass(tmp_ch, CLASS_PALADIN) && IS_GOOD(tmp_ch) && IS_GOOD(ch))
	      GET_ALIGNMENT(ch) = 0;
	    /* end good mob_good paladin check */
	    hit(tmp_ch, ch, 0);
	    return;
	  }
	  
	  if(ch->equipment[WIELD] &&
	     ((ch->equipment[WIELD]->obj_flags.value[3] == 11) ||
	      (ch->equipment[WIELD]->obj_flags.value[3] == 1) ||
	      (ch->equipment[WIELD]->obj_flags.value[3] == 10)))
	    hit(ch, tmp_ch, SKILL_BACKSTAB);
	  else
	    hit(ch, tmp_ch, 0);
	  return;
	}
	
      }
      if (IS_SET(ch->specials.mob_act, ACT_META_AGG)) {
	tmp_ch = FindMetaVictim(ch);
	if (tmp_ch) {
	  if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n\r")) {
	    act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	  
	  /* check to see if the player is aggressive towards aggressive mobs */
	  if (IS_SET(tmp_ch->specials.flags, PLR_AGGR)
	      && (!tmp_ch->specials.fighting)) {
	    act("$n fumes with rage and attacks $N for no reason!",TRUE,tmp_ch,0,ch,TO_ROOM);
	    
	    act("You fume with rage and heroicly attack $N!",FALSE,tmp_ch,0,ch,TO_CHAR);
	    /* check to see if good mob attacks good paladin and makes
	       the mob neutral */
	    if (HasClass(tmp_ch, CLASS_PALADIN) && IS_GOOD(tmp_ch) && IS_GOOD(ch))
	      GET_ALIGNMENT(ch) = 0;
	    /* end good mob_good paladin check */
	    hit(tmp_ch, ch, 0);
	    return;
	  }
	  
	  if(ch->equipment[WIELD] &&
	     ((ch->equipment[WIELD]->obj_flags.value[3] == 11) ||
	      (ch->equipment[WIELD]->obj_flags.value[3] == 1) ||
	      (ch->equipment[WIELD]->obj_flags.value[3] == 10)))
	    hit(ch, tmp_ch, SKILL_BACKSTAB);
	  else
	    hit(ch, tmp_ch, 0);
	  return;
	}
      }
      
      if (IS_SET(ch->specials.mob_act, ACT_GUARDIAN) &&
	  mobile_guardian(ch))
	return;
    }
  
  if(ch->player.sounds && mobile_sounds(ch))
    return;
}
  
  
  
int SameRace( struct char_data *ch1, struct char_data *ch2)
{    
    if ((!ch1) || (!ch2))
      return(FALSE);
    
    if (ch1 == ch2)
      return(TRUE);
    
    if (IS_NPC(ch1) && (IS_NPC(ch2)))
      if (mob_index[ch1->nr].virt ==
	  mob_index[ch2->nr].virt) {
	return (TRUE);
      }
    
    if (GET_RACE(ch1) == GET_RACE(ch2)) {
      return(TRUE);
    }
    
    if (in_group(ch1,ch2))
      return(TRUE);
    
    return(FALSE);
}
  
int AssistFriend( struct char_data *ch)
{
  struct char_data *damsel, *targ, *tmp_ch, *next;
  int t, found;
  
  damsel = 0;
  targ = 0;
  
  if (check_peaceful(ch, ""))
    return FALSE;

  if (ch->in_room <= NOWHERE) {
    log_msg("AssistFriend: Sending character to the void");
    char_to_room(ch, 0);
    return FALSE;
  }
  
  /*
    find the people who are fighting
    */
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=next) {
    next = tmp_ch->next_in_room;
    if (CAN_SEE(ch,tmp_ch)) {
      if (!IS_SET(ch->specials.mob_act, ACT_WIMPY)) {
	if (MobFriend(ch, tmp_ch)) {
	  if (tmp_ch->specials.fighting)
	  {
	    damsel = tmp_ch;
	    /* PAC - we can exit early */
	    break;
	  }
	}
      }
    }
  }
  
  if (damsel) {
    /*
      check if the people in the room are fighting.
      */
    found = FALSE;
    for (t=1; t<=8 && !found;t++) {
      targ = FindAnAttacker(damsel);
      if (targ) {
	if (targ->specials.fighting)
	{
	    found = TRUE;
	    /* PAC - We can exit early  don't wait for the "for' test */
	    break;
	}
      }
    }
    if (targ) {
      if (targ->in_room == ch->in_room) {
	if (!IS_AFFECTED(ch, AFF_CHARM) ||
	    ch->master != targ) {
	  hit(ch,targ,0);
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}
  
void FindABetterWeapon(struct char_data *mob)
{
  struct obj_data *o, *best;
  /*
    pick up and wield weapons
    Similar code for armor, etc.
    */
  
  /* check whether this mob can wield */
  if (!HasHands(mob)) return;
  
  if (!real_roomp(mob->in_room)) return;
  
  /* check room */
  best = 0;
  for (o = real_roomp(mob->in_room)->contents; o; o = o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
	best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
	best = o;
      }
    }
  }
  /* check inv */
  for (o = mob->carrying; o; o=o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
	best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
	best = o;
      }
    }
  }
  
  if (mob->equipment[WIELD]) {
    if (best) {
       if (GetDamage(mob->equipment[WIELD],mob) >= GetDamage(best,mob)) {
          best = mob->equipment[WIELD];
       }
    } else {
      best = mob->equipment[WIELD];
    }
  }

  if (best) {
     if (GetHandDamage(mob) > GetDamage(best, mob)) {
        best = 0;
     }
  } else {
    return;  /* nothing to choose from */
  }

  if (best) {
      /*
	out with the old, in with the new
      */
      if (best->carried_by == mob) {
	 if (mob->equipment[WIELD]) {
            do_remove(mob, OBJ_NAME(mob->equipment[WIELD]), 0);
	 }
         do_wield(mob, OBJ_NAME(best), 0);
      } else if (best->equipped_by == mob) {
	/* do nothing */
	  return;
      } else {
         do_get(mob, OBJ_NAME(best), 0);
      }      
  } else {
    if (mob->equipment[WIELD]) {
      do_remove(mob, OBJ_NAME(mob->equipment[WIELD]), 0);
    }
  }
}
      
int GetDamage(struct obj_data *w, struct char_data *ch) 
{
  int ave;

  /*
   * return the average damage of the weapon, with plusses.
   */

  ave = (int) (((double) w->obj_flags.value[2]/2.0 + 0.5) *
	       (double) w->obj_flags.value[1]);
  
  ave += GetDamBonus(w);

  /*
    check for immunity:
    */
  if (ch->specials.fighting) {
    ave = PreProcDam(ch->specials.fighting, ITEM_TYPE(w), ave);
    ave = WeaponCheck(ch, ch->specials.fighting, ITEM_TYPE(w), ave);
  }
  return(ave);
}
      
int GetDamBonus(struct obj_data *w)
{
   int j, tot=0;

    /* return the damage bonus from a weapon */
   for(j=0; j<MAX_OBJ_AFFECT; j++) {
      if (w->affected[j].location == APPLY_DAMROLL || 
	  w->affected[j].location == APPLY_HITNDAM) {
	  tot += w->affected[j].modifier;	
	}
    }
    return(tot);
}
	      
int GetHandDamage(struct char_data *ch) 
{
  int num, size, ave;

  /*
    return the hand damage of the weapon, with plusses.
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice);

    */

  num  = ch->specials.damnodice;
  size = ch->specials.damsizedice;
  
  ave = (int) (((double) size/2.0 + 0.5) * (double) num);

  /*
    check for immunity:
    */
  if (ch->specials.fighting) {
    ave = PreProcDam(ch->specials.fighting, TYPE_HIT, ave);
    ave = WeaponCheck(ch, ch->specials.fighting, TYPE_HIT, ave);
  }
  return(ave);
}

/*
  check to see if a mob is a friend

*/


int MobFriend( struct char_data *ch, struct char_data *f)
{
    if (SameRace(ch, f)) {
	if (IS_GOOD(ch)) {
	    if (IS_GOOD(f)) {
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	} else if (IS_NPC(f))
	    return(TRUE);
    }
    return(FALSE);
}

int mobile_sounds(struct char_data* ch)
{
    char buf[256];
    
    if(ch->player.sounds && !IS_PC(ch) && (number(0, 5) == 0))
    {
	if (ch->specials.default_pos > POSITION_SLEEPING)
	{
	    if (GET_POS(ch) > POSITION_SLEEPING)
		MakeNoise(ch->in_room, ss_data(ch->player.sounds),
			  ss_data(ch->player.distant_snds));
	    else if (GET_POS(ch) == POSITION_SLEEPING)
	    {
		/*
		 * snore 
		 */	 
		sprintf(buf, "%s snores loudly.\n\r", GET_NAME(ch));
		MakeNoise(ch->in_room, buf, 
			  "You hear a loud snore nearby.\n\r");
	    }
	}
	else if (GET_POS(ch) == ch->specials.default_pos)
	{
	    /*
	     * Make the sound
	     */       
	    MakeNoise(ch->in_room,
		      ss_data(ch->player.sounds),
		      ss_data(ch->player.distant_snds));
	}

	return 1;
    }

    return 0;
}
