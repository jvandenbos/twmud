/*
  Various utilities for handling traps
*/

#include "config.h"

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "trap.h"
#include "comm.h"
#include "db.h"
#include "constants.h"
#include "multiclass.h"
#include "utility.h"
#include "fight.h"
#include "spell_util.h"
#include "opinion.h"
#include "handler.h"
#include "act.h"
#include "recept.h"
#include "magicutils.h"
#include "scrap.h"
#include <string.h>

/* forward declarations */
int TriggerTrap( struct char_data *ch, struct obj_data *i);


void do_settrap( struct char_data *ch, char *arg, int cmd)
{

  /* parse for directions */

/* trap that affects all directions is an AE trap */

  /* parse for type       */
  /* parse for level      */

}

int CheckForMoveTrap(struct char_data *ch, int dir)
{
  struct obj_data *i;

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(i),TRAP_EFF_MOVE)) &&
	(GET_TRAP_CHARGES(i) > 0))
        if (IS_SET(GET_TRAP_EFF(i), TrapDir[dir])) 
	   return(TriggerTrap(ch, i));
  }
  return(FALSE);
}

int CheckForInsideTrap(struct char_data *ch, struct obj_data *i)
{
  struct obj_data *t;

  for (t = i->contains; t; t = t->next_content) {
    if ((ITEM_TYPE(t) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(t),TRAP_EFF_OBJECT)) &&
	(GET_TRAP_CHARGES(t) > 0)) {
	   return(TriggerTrap(ch, t));  
	 }
  }
  return(FALSE);
}

int CheckForAnyTrap(struct char_data *ch, struct obj_data *i)
{
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(GET_TRAP_CHARGES(i) > 0))
	   return(TriggerTrap(ch, i));

  return(FALSE);
}



int CheckForGetTrap(struct char_data *ch, struct obj_data *i)
{
    if ((ITEM_TYPE(i) == ITEM_TRAP) && 
	(IS_SET(GET_TRAP_EFF(i),TRAP_EFF_OBJECT)) &&
	(GET_TRAP_CHARGES(i) > 0)) {
	   return(TriggerTrap(ch, i));  
	 }
    return(FALSE);
}



int TriggerTrap( struct char_data *ch, struct obj_data *i)
{
  int adj, fireperc, roll;
  struct char_data *v, *nv;

  if (ITEM_TYPE(i) == ITEM_TRAP) {
    if (i->obj_flags.value[TRAP_CHARGES]) {
	adj = GET_TRAP_LEV(i) - GetMaxLevel(ch);
	adj -= dex_app[GET_DEX(ch)].reaction * 5;
	fireperc = 95 + adj;
	roll = number(1,100);
	if (roll < fireperc) {   /* trap is sprung */
	act("You hear a strange noise...", TRUE, ch, 0, 0, TO_ROOM);
	act("You hear a strange noise...", TRUE, ch, 0, 0, TO_CHAR);
	  GET_TRAP_CHARGES(i) -= 1;
	  if (IS_SET(GET_TRAP_EFF(i),TRAP_EFF_ROOM)) {
	    for (v = real_roomp(ch->in_room)->people;v;v = nv) {
		nv = v->next_in_room;
	      FindTrapDamage(v,i);
	    }	    
	  } else {
	    FindTrapDamage(ch,i);
	  }
	return(TRUE);
       }
     }
  }
  return(FALSE);
}

void FindTrapDamage( struct char_data *v, struct obj_data *i)
{
/*
   trap types < 0 are special
*/

  if (GET_TRAP_DAM_TYPE(i) >= 0) {
    TrapDamage(v,GET_TRAP_DAM_TYPE(i),3*GET_TRAP_LEV(i),i);
  } else {
     TrapDamage(v, GET_TRAP_DAM_TYPE(i), 0,i);
  }

}

void TrapDamage(struct char_data *v, int damtype, int amnt, struct obj_data *t)
{
  char buf[132];

  amnt = SkipImmortals(v, amnt);
  if (amnt == -1) {
    return;
  }

  if (IS_AFFECTED(v, AFF_SANCTUARY))
       	   amnt = MAX((int)(amnt/2), 0);  /* Max 1/2 damage when sanct'd */
        
   amnt = PreProcDam(v, damtype, amnt);

   if (saves_spell(v, SAVING_PETRI, 0))
     amnt = MAX((int)(amnt/2),0);

   DamageStuff(v, damtype, amnt);

   amnt=MAX(amnt,0);

   GET_HIT(v)-=amnt;

   update_pos(v);

   TrapDam(v, damtype, amnt, t);

   InformMess(v);
   if (GET_POS(v) == POSITION_DEAD) {
      if (!IS_NPC(v)) {
      	  if (real_roomp(v->in_room)->name)
       	     sprintf(buf, "%s killed by a trap at %s",
	       		GET_NAME(v),
	       		real_roomp(v->in_room)->name);
	      log_msg(buf);

			/* remove the hatreds of this character */
       	  }

       die(v);
    }
} 

void TrapDam(struct char_data *v, int damtype, int amnt, struct obj_data *t)
{

  char desc[20];
  char buf[132];

  /* easier than dealing with message(ug) */
  switch(damtype) {
  case TRAP_DAM_PIERCE:
    strcpy(desc,"pierced");
    break;
  case TRAP_DAM_SLASH:
    strcpy(desc,"sliced");
    break;
  case TRAP_DAM_BLUNT:
    strcpy(desc,"pounded");
    break;
  case TRAP_DAM_FIRE:
    strcpy(desc,"seared");
    break;
  case TRAP_DAM_COLD:
    strcpy(desc, "frozen");
    break;
  case TRAP_DAM_ACID:
    strcpy(desc, "corroded");
    break;
  case TRAP_DAM_ENERGY:
    strcpy(desc, "blasted");
    break;
  case TRAP_DAM_SLEEP:
    strcpy(desc, "knocked out");
    break;
  case TRAP_DAM_TELEPORT:
    strcpy(desc, "transported");
    break;
  default:
    strcpy(desc, "blown away");
    break;
  }
   
  if ((damtype != TRAP_DAM_TELEPORT)   &&
      (damtype != TRAP_DAM_SLEEP)) {
      if (amnt > 0) {
    sprintf(buf, "$n is %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_ROOM);
    sprintf(buf, "You are %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_CHAR);
      } else {
    sprintf(buf, "$n is almost %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_ROOM);
    sprintf(buf, "You are almost %s by $p!", desc);
    act(buf,TRUE,v,t,0,TO_CHAR);
      }
  }

  if (damtype == TRAP_DAM_TELEPORT) {
    TrapTeleport(v);
  } else if (damtype == TRAP_DAM_SLEEP) {
    TrapSleep(v);
  }

}


void TrapTeleport(struct char_data *v) 
{
  int to_room;

  if (saves_spell(v,SAVING_SPELL, 0)) {
    send_to_char("You feel strange, but the effect fades.\n\r",v);
    return;
  } 

  do {
    to_room = number(0, top_of_world);
  } while (IS_SET(real_roomp(to_room)->room_flags, NO_TRAVEL_IN));

  act("$n slowly fade out of existence.", FALSE, v,0,0,TO_ROOM);
  char_from_room(v);
  char_to_room(v, to_room);
  act("$n slowly fade in to existence.", FALSE, v,0,0,TO_ROOM);

  do_look(v, "", 0);

  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && !IS_GOD(v))
    do_death_trap(v);
}

void TrapSleep(struct char_data *v)
{

  struct affected_type af;

  if ( !saves_spell(v, SAVING_SPELL, IMM_SLEEP) )  {
    af.type      = SPELL_SLEEP;
    af.duration  = 12;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    af.mana_cost = 0;
    af.caster	 = 0;
    affect_join(v, &af, FALSE, FALSE);

    if (GET_POS(v)>POSITION_SLEEPING)    {
      act("You feel very sleepy ..... zzzzzz",FALSE,v,0,0,TO_CHAR);
      act("$n goes to sleep.",TRUE,v,0,0,TO_ROOM);
	    GET_POS(v)=POSITION_SLEEPING;
    }
  } else {
    send_to_char("You feel sleepy,but you recover\n\r",v);
  }

}


void InformMess( struct char_data *v)
{

  switch (GET_POS(v)) {
    case POSITION_MORTALLYW:
       	act("$n is mortally wounded.", 
	    TRUE, v, 0, 0, TO_ROOM);
       	act("You are mortally wounded.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
     case POSITION_INCAP:
       	act("$n is incapacitated.", 
	    TRUE, v, 0, 0, TO_ROOM);
	act("You are incapacitated.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
     case POSITION_STUNNED:
       	act("$n is stunned.", 
	    TRUE, v, 0, 0, TO_ROOM);
	act("You're stunned.", 
	    FALSE, v, 0, 0, TO_CHAR);
	break;
      case POSITION_DEAD:
       	act("$n is dead! R.I.P.", TRUE, v, 0, 0, TO_ROOM);
       	act("You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
       	break;
      default:  /* >= POSITION SLEEPING */
       	break;
      }
}

void do_death_trap(struct char_data* v)
{
    struct char_data* tmp;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s hit a death trap: %ld", GET_NAME(v), v->in_room);
    log_msg(buf);
/* JANWORK */
    if((tmp = v->specials.mounted_on)) {
	v->specials.mounted_on = 0;
	tmp->specials.ridden_by = 0;
	do_death_trap(tmp);
    }
    if((tmp = v->specials.ridden_by)) {
	v->specials.ridden_by = 0;
	tmp->specials.mounted_on = 0;
	do_death_trap(tmp);
    }

    death_cry(v);
    if (IS_SET(v->specials.mob_act, ACT_POLYSELF)) {
	/*
	 *   take char from storage, to room     
	 */
	tmp = pop_character(v);
	char_from_room(tmp);
	char_to_room(tmp, v->in_room);
	SwitchStuff(v, tmp);
	extract_char(v);
	v = tmp;
    }

    StuffToRoom(v, v->in_room);
    SaveChar(v, AUTO_RENT, TRUE);

    extract_char(v);
}
